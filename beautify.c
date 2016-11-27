/**
 * Copyright (C) 2012 hejian <hejian.he@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "beautify-effect.h"

#define PLUG_IN_PROC   "plug-in-beautify"
#define PLUG_IN_BINARY "beautify"
#define PLUG_IN_ROLE   "gimp-beautify"

#define PREVIEW_SIZE  480
#define THUMBNAIL_SIZE  80

typedef struct
{
  gint brightness;
  gint contrast;
  gdouble saturation;
  gdouble definition;
  gdouble hue;
  gdouble cyan_red;
  gdouble magenta_green;
  gdouble yellow_blue;

  BeautifyEffectType effect;
  gdouble opacity;
} BeautifyValues;

static const BeautifyEffectType basic_effects[] =
{
  BEAUTIFY_EFFECT_SOFT_LIGHT,
  BEAUTIFY_EFFECT_WARM,
  BEAUTIFY_EFFECT_SHARPEN,
  BEAUTIFY_EFFECT_SOFT,
  BEAUTIFY_EFFECT_STRONG_CONTRAST,
  BEAUTIFY_EFFECT_SMART_COLOR,
  BEAUTIFY_EFFECT_BLACK_AND_WHITE,
  BEAUTIFY_EFFECT_INVERT,
};

static const BeautifyEffectType lomo_effects[] =
{
  BEAUTIFY_EFFECT_CLASSIC_LOMO,
  BEAUTIFY_EFFECT_RETRO_LOMO,
  BEAUTIFY_EFFECT_GOTHIC_STYLE,
  BEAUTIFY_EFFECT_FILM,
  BEAUTIFY_EFFECT_HDR,
  BEAUTIFY_EFFECT_CLASSIC_HDR,
  BEAUTIFY_EFFECT_YELLOWING_DARK_CORNERS,
  BEAUTIFY_EFFECT_IMPRESSION,
  BEAUTIFY_EFFECT_DEEP_BLUE_TEAR_RAIN,
  BEAUTIFY_EFFECT_PURPLE_SENSATION,
  BEAUTIFY_EFFECT_BRONZE,
  BEAUTIFY_EFFECT_RECALL,
};

static const BeautifyEffectType studio_effects[] =
{
  BEAUTIFY_EFFECT_ELEGANT,
  BEAUTIFY_EFFECT_LITTLE_FRESH,
  BEAUTIFY_EFFECT_CLASSIC_STUDIO,
  BEAUTIFY_EFFECT_RETRO,
  BEAUTIFY_EFFECT_PINK_LADY,
  BEAUTIFY_EFFECT_ABAO_COLOR,
  BEAUTIFY_EFFECT_ICE_SPIRIT,
  BEAUTIFY_EFFECT_JAPANESE_STYLE,
  BEAUTIFY_EFFECT_NEW_JAPANESE_STYLE,
  BEAUTIFY_EFFECT_MILK,
  BEAUTIFY_EFFECT_OLD_PHOTOS,
  BEAUTIFY_EFFECT_WARM_YELLOW,
  BEAUTIFY_EFFECT_BLUES,
  BEAUTIFY_EFFECT_COLD_BLUE,
  BEAUTIFY_EFFECT_COLD_GREEN,
  BEAUTIFY_EFFECT_PURPLE_FANTASY,
  BEAUTIFY_EFFECT_COLD_PURPLE,
};

static const BeautifyEffectType fashion_effects[] =
{
  BEAUTIFY_EFFECT_BRIGHT_RED,
  BEAUTIFY_EFFECT_CHRISTMAS_EVE,
  BEAUTIFY_EFFECT_NIGHT_VIEW,
  BEAUTIFY_EFFECT_ASTRAL,
  BEAUTIFY_EFFECT_COLORFUL_GLOW,
  BEAUTIFY_EFFECT_PICK_LIGHT,
  BEAUTIFY_EFFECT_GLASS_DROPS,
};

static const BeautifyEffectType art_effects[] =
{
  BEAUTIFY_EFFECT_SKETCH,
  BEAUTIFY_EFFECT_LIFE_SKETCH,
  BEAUTIFY_EFFECT_CLASSIC_SKETCH,
  BEAUTIFY_EFFECT_COLOR_PENCIL,
  BEAUTIFY_EFFECT_TV_LINES,
  BEAUTIFY_EFFECT_RELIEF,
};

static const BeautifyEffectType gradient_effects[] =
{
  BEAUTIFY_EFFECT_BEAM_GRADIENT,
  BEAUTIFY_EFFECT_SUNSET_GRADIENT,
  BEAUTIFY_EFFECT_RAINBOW_GRADIENT,
  BEAUTIFY_EFFECT_PINK_PURPLE_GRADIENG,
  BEAUTIFY_EFFECT_PINK_BLUE_GRADIENT,
};

static void     query    (void);
static void     run      (const gchar      *name,
                          gint              nparams,
                          const GimpParam  *param,
                          gint             *nreturn_vals,
                          GimpParam       **return_vals);

static void     beautify        (GimpDrawable *drawable);
static void     beautify_effect (GimpDrawable *drawable);

static gboolean beautify_dialog (gint32        image_ID,
                                 GimpDrawable *drawable);

static void     create_base_page (GtkNotebook *notebook);
static void     create_color_page (GtkNotebook *notebook);

static void     brightness_update    (GtkRange *range, gpointer data);
static void     contrast_update      (GtkRange *range, gpointer data);
static void     saturation_update    (GtkRange *range, gpointer data);
static void     definition_update    (GtkRange *range, gpointer data);
static void     hue_update           (GtkRange *range, gpointer data);
static void     cyan_red_update      (GtkRange *range, gpointer data);
static void     magenta_green_update (GtkRange *range, gpointer data);
static void     yellow_blue_update   (GtkRange *range, gpointer data);

static void     adjustment(gint32 image);

static void     reset_pressed (GtkButton *button, gpointer user_date);

static void     preview_update (GtkWidget *preview);

static GtkWidget* effect_option_new ();
static void       effect_opacity_update (GtkRange *range, gpointer data);

static void create_effect_pages (GtkNotebook *notebook);
static void create_effect_page  (GtkNotebook *notebook, gchar *str);
static void effects_switch_page (GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);

static GtkWidget* effect_icon_new (BeautifyEffectType effect);

static gboolean select_effect (GtkWidget *widget, GdkEvent *event, gpointer user_data);

static void reset_adjustment ();

static void apply_effect ();
static void cancel_effect ();

const GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static BeautifyValues bvals =
{
  0,  /* brightness */
  0,  /* contrast   */
  0,  /* saturation */
  0,  /* definition */
  0,  /* hue */
  0,  /* cyan_red */
  0,  /* magenta_green */
  0,  /* yellow_blue */

  BEAUTIFY_EFFECT_NONE, /* effect */
  100,  /* opacity */
};

static gint32     image_ID         = 0;
static gint       width;
static gint       height;

static GtkWidget *brightness = NULL;
static GtkWidget *contrast = NULL;
static GtkWidget *saturation = NULL;
static GtkWidget *definition = NULL;
static GtkWidget *hue = NULL;
static GtkWidget *cyan_red = NULL;
static GtkWidget *magenta_green = NULL;
static GtkWidget *yellow_blue = NULL;

static GtkWidget *preview          = NULL;
static gint32     real_image       = 0;
static gint32     preview_image    = 0;
static gint32     saved_image      = 0;
static gint32     thumbnail        = 0;
/* a cache for preview_image, avoid redundant resize when press reset button */
static gint32     preview_image_cache = 0;

static GtkWidget *effect_option    = NULL;
static GtkWidget *effect_opacity   = NULL;

static BeautifyEffectType current_effect = BEAUTIFY_EFFECT_NONE;
gint32 preview_effect_layer = 0;

/* compatable with gtk2 */
#if GTK_MAJOR_VERSION < 3
GtkWidget *
gtk_box_new (GtkOrientation  orientation,
             gint            spacing)
{
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    return gtk_hbox_new (FALSE, spacing);
  else
    return gtk_vbox_new (FALSE, spacing);
}
#endif

MAIN ()

static void
query (void)
{
  static const GimpParamDef args[] =
  {
    { GIMP_PDB_INT32,    "run-mode",   "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { GIMP_PDB_IMAGE,    "image",      "Input image" },
    { GIMP_PDB_DRAWABLE, "drawable",   "Input drawable" },
    { GIMP_PDB_INT32,    "effect",     "The effect to apply { SOFT_LIGHT (1), WARM (2), SHARPEN (3), SOFT (4), REMOVE_FOG (5), STRONG_CONTRAST (6), SMART_COLOR (7), FULL_COLOR (8), BLACK_AND_WHITE (9), INVERT (10), NOISE (11), CLASSIC_LOMO (12), RETRO_LOMO (13), GOTHIC_STYLE (14), FILM (15), HDR (16), CLASSIC_HDR (17), REVERSE_COLOR (18) TIME_TUNNEL (19), AFTER_THE_YOUTH (20), AXIS_LENS (21), YELLOWING_DARK_CORNERS (22), OLD_DAYS (23), NOSTALGIA (24), IMPRESSION (25), DEEP_BLUE_TEAR_RAIN (26), PURPLE_SENSATION (27), BRONZE (28), COOL_COLORS (29), RECALL (30), RETRO_BLACK_AND_WHITE (31), COLORFUL_LOMO (32), ELEGANT (33), LITTLE_FRESH (34), CLASSIC_STUDIO (35), RETRO (36), PINK_LADY (37), ABAO_COLOR (38), ICE_SPIRIT (39), MATTE_FINISH_STYLE (40), JAPANESE_STYLE (41), NEW_JAPANESE_STYLE (42), MILK (43), FLEETING_TIME (44), CLOUD (45), OLD_PHOTOS (46), WARM_YELLOW (47), BLUES (48), COLD_BLUE (49), COLD_GREEN (50), PURPLE_FANTASY (51), COLD_PURPLE (52), BRIGHT_RED (53), CHRISTMAS_EVE (54), SNOW (55), NIGHT_VIEW (56), ASTRAL (57), COLORFUL_GLOW (58), BACKLIGHT (59), PICK_LIGHT (60), COLORFUL (61), HEART_SHAPED_HALO (62), BUBBLES_GRADIENT (63), GLASS_DROPS (64), NEW_YEAR_ATMOSPHERE (65), SKETCH (66), LIFE_SKETCH (67), CLASSIC_SKETCH (68), COLOR_PENCIL (69), TV_LINES (70), BLACK_AND_WHITE_NEWSPAPER (71), RELIEF (72), BEAUTIFY_EFFECT_PAINTING (73), BEAM_GRADIENT (74), SUNSET_GRADIENT (75), COOL_GRADIENT (76), BLUE_YELLOW_GRADIENT (77), RAINBOW_GRADIENT (78), FOUR_COLOR_GRADIENT (79), PINK_PURPLE_GRADIENG (80), PINK_BLUE_GRADIENT (81) }" },
    { GIMP_PDB_FLOAT,    "opacity",   "The effect opacity (0 <= opacity <= 100)" }
  };

  gimp_install_procedure (PLUG_IN_PROC,
                          "Quickly and easily beautify the photo.",
                          "Quickly and easily beautify the photo. Goto https://github.com/hejiann/beautify/wiki get more help.",
                          "Hejian <hejian.he@gmail.com>",
                          "Hejian <hejian.he@gmail.com>",
                          "2012",
                          "_Beautify...",
                          "RGB*, GRAY*",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Beautify");
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[2];
  GimpDrawable      *drawable;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = status;

  image_ID = param[1].data.d_image;
  drawable = gimp_drawable_get (param[2].data.d_drawable);

  width = gimp_image_width (image_ID);
  height = gimp_image_height (image_ID);

  switch (run_mode)
  {
    case GIMP_RUN_INTERACTIVE:
      if (! beautify_dialog (image_ID, drawable))
        return;
      break;

    case GIMP_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 5)
        status = GIMP_PDB_CALLING_ERROR;

      if (status == GIMP_PDB_SUCCESS)
      {
        bvals.effect = param[3].data.d_int32;
        bvals.opacity = param[4].data.d_float;
      }
      break;

    case GIMP_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      gimp_get_data (PLUG_IN_PROC, &bvals);
      break;

    default:
      break;
  }

  if ((status == GIMP_PDB_SUCCESS) &&
      (gimp_drawable_is_rgb(drawable->drawable_id) ||
       gimp_drawable_is_gray(drawable->drawable_id)))
    {
      /* Run! */
      gimp_image_undo_group_start (image_ID);
      if (run_mode == GIMP_RUN_INTERACTIVE)
        beautify (drawable);
      else
        beautify_effect (drawable);
      gimp_image_undo_group_end (image_ID);

      /* If run mode is interactive, flush displays */
      if (run_mode != GIMP_RUN_NONINTERACTIVE)
        gimp_displays_flush ();

      /* Store data */
      if (run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PLUG_IN_PROC, &bvals, sizeof (BeautifyValues));

    }

  gimp_drawable_detach (drawable);
}

static void
beautify (GimpDrawable *drawable)
{
  apply_effect ();
  gint32 source = gimp_image_get_active_layer (real_image);
  gimp_edit_copy (source);
  gint32 floating_sel = gimp_edit_paste (drawable->drawable_id, FALSE);
  gimp_floating_sel_anchor (floating_sel);
}

static void
beautify_effect (GimpDrawable *drawable)
{
  run_effect (image_ID, bvals.effect);

  gint32 layer = gimp_image_get_active_layer (image_ID);
  gimp_layer_set_opacity (layer, bvals.opacity);

  gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_IMAGE);
}

static gint32
image_copy_scale (gint32 src_image,
                  gint max_size)
{
  gint32 image = gimp_image_duplicate(src_image);

  if (width > max_size && height > max_size) {
    if (width > height)
    {
      gimp_image_scale (image, max_size * width / height, max_size);
    }
    else
    {
      gimp_image_scale (image, max_size, max_size * height / width);
    }
  }
  return image;
}

static gboolean
beautify_dialog (gint32        image_ID,
                 GimpDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_hbox;
  GtkWidget *left_vbox;
  GtkWidget *middle_vbox;
  GtkWidget *right_vbox;

  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = gimp_dialog_new ("Beautify", PLUG_IN_ROLE,
                            NULL, 0,
                            gimp_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gimp_window_set_transient (GTK_WINDOW (dialog));

  gtk_widget_show (dialog);

  main_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_hbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_hbox, TRUE, TRUE, 0);
  gtk_widget_show (main_hbox);

  left_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (left_vbox), 12);
  gtk_box_pack_start (GTK_BOX (main_hbox), left_vbox, TRUE, TRUE, 0);
  gtk_widget_show (left_vbox);

  middle_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (middle_vbox), 12);
  gtk_box_pack_start (GTK_BOX (main_hbox), middle_vbox, TRUE, TRUE, 0);
  gtk_widget_show (middle_vbox);

  right_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (right_vbox), 12);
  gtk_box_pack_start (GTK_BOX (main_hbox), right_vbox, TRUE, TRUE, 0);
  gtk_widget_show (right_vbox);

  /* adjustment */
  GtkWidget *notebook = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (left_vbox), notebook, FALSE, FALSE, 0);
  gtk_widget_show (notebook);

  create_base_page (GTK_NOTEBOOK (notebook));
  create_color_page (GTK_NOTEBOOK (notebook));

  /* buttons */
  GtkWidget *buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_pack_start (GTK_BOX (middle_vbox), buttons, FALSE, FALSE, 0);
  gtk_widget_show (buttons);

  GtkWidget *reset = gtk_button_new_with_label ("Reset");
  gtk_box_pack_start (GTK_BOX (buttons), reset, FALSE, FALSE, 0);
  gtk_widget_show (reset);
  g_signal_connect (reset, "pressed", G_CALLBACK (reset_pressed), NULL);

  real_image = gimp_image_duplicate (image_ID);
  /* preview */
  preview_image_cache = image_copy_scale (real_image, PREVIEW_SIZE);
  preview_image = gimp_image_duplicate(preview_image_cache);
  /* create thumbnail cache for effect icon */
  thumbnail = image_copy_scale (preview_image, THUMBNAIL_SIZE);

  preview = gtk_image_new();
  preview_update (preview);

  gtk_box_pack_start (GTK_BOX (middle_vbox), preview, FALSE, FALSE, 0);
  gtk_widget_show (preview);

  /* effect option */
  effect_option = effect_option_new ();
  gtk_box_pack_start (GTK_BOX (middle_vbox), effect_option, FALSE, FALSE, 0);

  /* effects */
  notebook = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (right_vbox), notebook, FALSE, FALSE, 0);
  gtk_widget_show (notebook);

  create_effect_pages (GTK_NOTEBOOK (notebook));

  gboolean run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gimp_image_delete(preview_image);
  gimp_image_delete(preview_image_cache);
  gimp_image_delete(thumbnail);
  gtk_widget_destroy (dialog);

  return run;
}

static void
create_base_page (GtkNotebook *notebook) {
  GtkWidget *label;
  GtkWidget *hscale;

  GtkWidget *pagelabel = gtk_label_new ("Basic");

  GtkWidget *thispage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 12);
  gtk_widget_show (thispage);

  /* brightness */
  label = gtk_label_new ("Brightness");
  gtk_box_pack_start (GTK_BOX (thispage), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  brightness = gtk_hscale_new_with_range (-127, 127, 1);
  gtk_range_set_value (GTK_RANGE (brightness), bvals.brightness);
  gtk_scale_set_value_pos (GTK_SCALE (brightness), GTK_POS_BOTTOM);
  gtk_box_pack_start (GTK_BOX (thispage), brightness, FALSE, FALSE, 0);
  gtk_widget_show (brightness);

  g_signal_connect (brightness, "value-changed",
                   G_CALLBACK (brightness_update),
                   NULL);

  /* contrast */
  label = gtk_label_new ("Contrast");
  gtk_box_pack_start (GTK_BOX (thispage), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  contrast = gtk_hscale_new_with_range (-50, 50, 1);
  gtk_range_set_value (GTK_RANGE (contrast), bvals.contrast);
  gtk_scale_set_value_pos (GTK_SCALE (contrast), GTK_POS_BOTTOM);
  gtk_box_pack_start (GTK_BOX (thispage), contrast, FALSE, FALSE, 0);
  gtk_widget_show (contrast);

  g_signal_connect (contrast, "value-changed",
                   G_CALLBACK (contrast_update),
                   NULL);

  /* saturation */
  label = gtk_label_new ("Saturation");
  gtk_box_pack_start (GTK_BOX (thispage), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  saturation = gtk_hscale_new_with_range (-50, 50, 1);
  gtk_range_set_value (GTK_RANGE (saturation), bvals.saturation);
  gtk_scale_set_value_pos (GTK_SCALE (saturation), GTK_POS_BOTTOM);
  gtk_box_pack_start (GTK_BOX (thispage), saturation, FALSE, FALSE, 0);
  gtk_widget_show (saturation);

  g_signal_connect (saturation, "value-changed",
                   G_CALLBACK (saturation_update),
                   NULL);

  /* definition */
  label = gtk_label_new ("Definition");
  gtk_box_pack_start (GTK_BOX (thispage), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  definition = gtk_hscale_new_with_range (-50, 50, 1);
  gtk_range_set_value (GTK_RANGE (definition), bvals.definition);
  gtk_scale_set_value_pos (GTK_SCALE (definition), GTK_POS_BOTTOM);
  gtk_box_pack_start (GTK_BOX (thispage), definition, FALSE, FALSE, 0);
  gtk_widget_show (definition);

  g_signal_connect (definition, "value-changed",
                   G_CALLBACK (definition_update),
                   NULL);

  gtk_notebook_append_page_menu (notebook, thispage, pagelabel, NULL);
}

static void adjustment_update () {
  adjustment (preview_image);
  preview_update (preview);
}

static void
brightness_update (GtkRange *range, gpointer data) {
  bvals.brightness = gtk_range_get_value (range);
  adjustment_update ();
}

static void
contrast_update (GtkRange *range, gpointer data) {
  bvals.contrast = gtk_range_get_value (range);
  adjustment_update ();
}

static void
saturation_update (GtkRange *range, gpointer data) {
  bvals.saturation = gtk_range_get_value (range);
  adjustment_update ();
}

static void
definition_update (GtkRange *range, gpointer data) {
  bvals.definition = gtk_range_get_value (range);
  adjustment_update ();
}

static void
create_color_page (GtkNotebook *notebook) {
  GtkWidget *label;
  GtkWidget *hscale;

  GtkWidget *pagelabel = gtk_label_new ("Color");

  GtkWidget *thispage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 12);
  gtk_widget_show (thispage);

  /* hue */
  label = gtk_label_new ("Hue");
  gtk_box_pack_start (GTK_BOX (thispage), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  hue = gtk_hscale_new_with_range (-180, 180, 1);
  gtk_range_set_value (GTK_RANGE (hue), bvals.hue);
  gtk_scale_set_value_pos (GTK_SCALE (hue), GTK_POS_BOTTOM);
  gtk_box_pack_start (GTK_BOX (thispage), hue, FALSE, FALSE, 0);
  gtk_widget_show (hue);

  g_signal_connect (hue, "value-changed",
                   G_CALLBACK (hue_update),
                   NULL);

  GtkWidget *table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (thispage), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /* cyan_red */
  GtkWidget *event_box;
  GdkColor color;

  event_box = gtk_event_box_new ();
  color.red = 0x0; color.green = 0xFFFF; color.blue = 0xFFFF;
  gtk_widget_modify_bg (event_box, 0, &color);

  label = gtk_label_new ("C");
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), event_box, 0, 1, 0, 1);
  gtk_widget_show (event_box);

  cyan_red = gtk_hscale_new_with_range (-50, 50, 1);
  gtk_range_set_value (GTK_RANGE (cyan_red), bvals.cyan_red);
  gtk_scale_set_value_pos (GTK_SCALE (cyan_red), GTK_POS_BOTTOM);
  gtk_table_attach_defaults (GTK_TABLE (table), cyan_red, 1, 2, 0, 1);
  gtk_widget_show (cyan_red);

  gtk_widget_set_size_request (cyan_red, 100, -1);

  g_signal_connect (cyan_red, "value-changed",
                   G_CALLBACK (cyan_red_update),
                   NULL);

  event_box = gtk_event_box_new ();
  color.red = 0xFFFF; color.green = 0x0; color.blue = 0x0;
  gtk_widget_modify_bg (event_box, 0, &color);

  label = gtk_label_new ("R");
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), event_box, 2, 3, 0, 1);
  gtk_widget_show (event_box);

  /* magenta_green */
  event_box = gtk_event_box_new ();
  color.red = 0xFFFF; color.green = 0x0; color.blue = 0xFFFF;
  gtk_widget_modify_bg (event_box, 0, &color);

  label = gtk_label_new ("M");
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), event_box, 0, 1, 1, 2);
  gtk_widget_show (event_box);

  magenta_green = gtk_hscale_new_with_range (-50, 50, 1);
  gtk_range_set_value (GTK_RANGE (magenta_green), bvals.magenta_green);
  gtk_scale_set_value_pos (GTK_SCALE (magenta_green), GTK_POS_BOTTOM);
  gtk_table_attach_defaults (GTK_TABLE (table), magenta_green, 1, 2, 1, 2);
  gtk_widget_show (magenta_green);

  g_signal_connect (magenta_green, "value-changed",
                   G_CALLBACK (magenta_green_update),
                   NULL);

  event_box = gtk_event_box_new ();
  color.red = 0x0; color.green = 0xFFFF; color.blue = 0x0;
  gtk_widget_modify_bg (event_box, 0, &color);

  label = gtk_label_new ("G");
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), event_box, 2, 3, 1, 2);
  gtk_widget_show (event_box);

  /* yellow_blue */
  event_box = gtk_event_box_new ();
  color.red = 0xFFFF; color.green = 0xFFFF; color.blue = 0x0;
  gtk_widget_modify_bg (event_box, 0, &color);

  label = gtk_label_new ("Y");
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), event_box, 0, 1, 2, 3);
  gtk_widget_show (event_box);

  yellow_blue = gtk_hscale_new_with_range (-50, 50, 1);
  gtk_range_set_value (GTK_RANGE (yellow_blue), bvals.yellow_blue);
  gtk_scale_set_value_pos (GTK_SCALE (yellow_blue), GTK_POS_BOTTOM);
  gtk_table_attach_defaults (GTK_TABLE (table), yellow_blue, 1, 2, 2, 3);
  gtk_widget_show (yellow_blue);

  g_signal_connect (yellow_blue, "value-changed",
                   G_CALLBACK (yellow_blue_update),
                   NULL);

  event_box = gtk_event_box_new ();
  color.red = 0x0; color.green = 0x0; color.blue = 0xFFFF;
  gtk_widget_modify_bg (event_box, 0, &color);

  label = gtk_label_new ("B");
  gtk_container_add (GTK_CONTAINER (event_box), label);
  gtk_widget_show (label);
  gtk_table_attach_defaults (GTK_TABLE (table), event_box, 2, 3, 2, 3);
  gtk_widget_show (event_box);

  gtk_notebook_append_page_menu (notebook, thispage, pagelabel, NULL);
}

static void
hue_update (GtkRange *range, gpointer data) {
  bvals.hue = gtk_range_get_value (range);
  adjustment_update ();
}

static void
cyan_red_update (GtkRange *range, gpointer data) {
  bvals.cyan_red = gtk_range_get_value (range);
  adjustment_update ();
}

static void
magenta_green_update (GtkRange *range, gpointer data) {
  bvals.magenta_green = gtk_range_get_value (range);
  adjustment_update ();
}

static void
yellow_blue_update (GtkRange *range, gpointer data) {
  bvals.yellow_blue = gtk_range_get_value (range);
  adjustment_update ();
}

static void
adjustment (gint32 image) {
  if (bvals.brightness == 0 && bvals.contrast == 0 && bvals.saturation == 0 && bvals.definition == 0 && bvals.hue == 0 && bvals.cyan_red == 0 && bvals.magenta_green == 0 && bvals.yellow_blue == 0)
    return;

  if (image == preview_image) {
    /* need to save previous image for preview,
     * since bvals should to apply origin image,
     * otherwise, they would accumulate and result in unwanted effect.
     */
    if (!saved_image) {
      gtk_widget_hide (effect_option);
      saved_image = gimp_image_duplicate (preview_image);
    }
    preview_image = gimp_image_duplicate (saved_image);
    image = preview_image;
  }
  gint32 layer = gimp_image_get_active_layer (image);

  if (bvals.brightness != 0 || bvals.contrast != 0)
  {
    //TODO
    //Fix Int to Double math
    gdouble low_input = 0.0;
    gdouble high_input = 1.0; //255
    gdouble low_output = 0.0;
    gdouble high_output = 1.0; //255

    //TODO
    //scale input using 0-100
    //previously done as -127 <-> 127
    if (bvals.brightness > 0)
      high_input -= bvals.brightness;
    if (bvals.brightness < 0)
      high_output += bvals.brightness;

    //TODO
    //scale input using 0-100
    //previously done as -127 <-> 127
    gint value = 62 * (bvals.contrast / 50.0);
    if (value > 0) {
      low_input += value;
      high_input -= value;
    }
    if (value < 0) {
      low_output -= value;
      high_output += value;
    }

    gimp_drawable_levels (layer, GIMP_HISTOGRAM_VALUE,
                 low_input, high_input,
                 1,
                 low_output, high_output);
  }

  if (bvals.saturation != 0 || bvals.hue)
    gimp_hue_saturation (layer, GIMP_ALL_HUES, bvals.hue, 0, bvals.saturation);

  if (bvals.definition > 0)
  {
    gint       nreturn_vals;
    GimpParam *return_vals;
    gint32     percent = 78 * (bvals.definition / 50);
    return_vals = gimp_run_procedure ("plug-in-sharpen",
                                      &nreturn_vals,
                                      GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
                                      GIMP_PDB_IMAGE, image_ID,
                                      GIMP_PDB_DRAWABLE, layer,
                                      GIMP_PDB_INT32, percent,
                                      GIMP_PDB_END);
    gimp_destroy_params (return_vals, nreturn_vals);
  }
  else if (bvals.definition < 0)
  {
    // TODO
  }

  if (bvals.cyan_red != 0 || bvals.magenta_green != 0 || bvals.yellow_blue != 0)
  {
    gimp_color_balance (layer, GIMP_SHADOWS, TRUE,
                        bvals.cyan_red, bvals.magenta_green, bvals.yellow_blue);
    gimp_color_balance (layer, GIMP_MIDTONES, TRUE,
                        bvals.cyan_red, bvals.magenta_green, bvals.yellow_blue);
    gimp_color_balance (layer, GIMP_HIGHLIGHTS, TRUE,
                        bvals.cyan_red, bvals.magenta_green, bvals.yellow_blue);
  }
}

static void
reset_pressed (GtkButton *button, gpointer user_date)
{
  reset_adjustment ();
  cancel_effect ();

  gimp_image_delete (real_image);
  gimp_image_delete (preview_image);
  real_image = gimp_image_duplicate (image_ID);
  preview_image = gimp_image_duplicate(preview_image_cache);
  preview_update (preview);
}

static void
preview_update (GtkWidget *preview)
{
  gint preview_size = PREVIEW_SIZE;
  gint max_size = height;
  if (height < width)
    max_size = width;
  if (preview_size > max_size)
    preview_size = max_size;
  GdkPixbuf *pixbuf = gimp_image_get_thumbnail (preview_image, preview_size, preview_size, GIMP_PIXBUF_SMALL_CHECKS);
  gtk_image_set_from_pixbuf (GTK_IMAGE(preview), pixbuf);
}

static GtkWidget*
effect_option_new () {
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

  GtkWidget *label = gtk_label_new ("Effect Opacity:");
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  effect_opacity = gtk_hscale_new_with_range (0, 100, 1);
  gtk_range_set_value (GTK_RANGE (effect_opacity), 100);
  gtk_scale_set_value_pos (GTK_SCALE (effect_opacity), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (box), effect_opacity, TRUE, TRUE, 0);
  gtk_widget_show (effect_opacity);

  g_signal_connect (effect_opacity, "value-changed",
                   G_CALLBACK (effect_opacity_update),
                   NULL);

  return box;
}

static void
effect_opacity_update (GtkRange *range, gpointer data) {
  if (current_effect == BEAUTIFY_EFFECT_NONE) {
    return;
  }

  gdouble opacity = gtk_range_get_value (range);
  gint32 layer = gimp_image_get_active_layer (preview_image);
  gimp_layer_set_opacity (layer, opacity);

  preview_update (preview);
}

static void
create_effect_pages (GtkNotebook *notebook) {
  create_effect_page (notebook, "Basic");
  create_effect_page (notebook, "LOMO");
  create_effect_page (notebook, "Studio");
  create_effect_page (notebook, "Fashion");
  create_effect_page (notebook, "Art");
  create_effect_page (notebook, "Gradient");

  g_signal_connect (notebook, "switch-page", G_CALLBACK (effects_switch_page), NULL);

  GtkWidget *page = gtk_notebook_get_nth_page (notebook, 0);
  effects_switch_page(notebook, page, 0, NULL);
}

static void
create_effect_page (GtkNotebook *notebook, gchar *str) {
  GtkWidget *pagelabel = gtk_label_new (str);

  GtkWidget *thispage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 12);
  gtk_widget_show (thispage);

  gtk_notebook_append_page_menu (notebook, thispage, pagelabel, NULL);
}

static void
effects_switch_page (GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data)
{
  static time_t effects_timestamp [] = {0, 0, 0, 0, 0, 0};

  if (effects_timestamp[page_num] > 0)
    return;

  // fix gtk2
  page = gtk_notebook_get_nth_page (notebook, page_num);

  const BeautifyEffectType* effects;
  guint n_effects;

  switch (page_num)
  {
    case 0:
    {
      effects = basic_effects;
      n_effects = G_N_ELEMENTS (basic_effects);
      break;
    }
    case 1:
    {
      effects = lomo_effects;
      n_effects = G_N_ELEMENTS (lomo_effects);
      break;
    }
    case 2:
    {
      effects = studio_effects;
      n_effects = G_N_ELEMENTS (studio_effects);
      break;
    }
    case 3:
    {
      effects = fashion_effects;
      n_effects = G_N_ELEMENTS (fashion_effects);
      break;
    }
    case 4:
    {
      effects = art_effects;
      n_effects = G_N_ELEMENTS (art_effects);
      break;
    }
    case 5:
    {
      effects = gradient_effects;
      n_effects = G_N_ELEMENTS (gradient_effects);
      break;
    }
  }

  gtk_container_set_border_width (GTK_CONTAINER (page), 0);
  gtk_widget_set_size_request (page, -1, 480);

  /* scrolled window */
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (page), scrolled_window, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_window);

  /* table */
  gint rows = 5;
  gint cols = 3;
  GtkWidget *table = gtk_table_new (rows, cols, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_container_set_border_width (GTK_CONTAINER (table), 10);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), table);
  gtk_widget_show (table);

  gint row = 1;
  gint col = 1;

  gint i;
  for (i = 0; i < n_effects; i++)
  {
    GtkWidget *icon = effect_icon_new (effects[i]);
    gtk_table_attach_defaults (GTK_TABLE (table), icon, col - 1, col, row - 1, row);
    gtk_widget_show (icon);

    col++;
    if (col > cols)
    {
      row++;
      col = 1;
    }
  }

  effects_timestamp[page_num] = time (NULL);
}

static GtkWidget *
effect_icon_new (BeautifyEffectType effect)
{
  gchar *title;

  switch (effect) {
    case BEAUTIFY_EFFECT_SOFT_LIGHT:
      title = "Soft Light";
      break;
    case BEAUTIFY_EFFECT_WARM:
      title = "Warm";
      break;
    case BEAUTIFY_EFFECT_SHARPEN:
      title = "Sharpen";
      break;
    case BEAUTIFY_EFFECT_SOFT:
      title = "Soft";
      break;
    case BEAUTIFY_EFFECT_STRONG_CONTRAST:
      title = "Strong\nContrast";
      break;
    case BEAUTIFY_EFFECT_SMART_COLOR:
      title = "Smart Color";
      break;
    case BEAUTIFY_EFFECT_BLACK_AND_WHITE:
      title = "Black\nand White";
      break;
    case BEAUTIFY_EFFECT_INVERT:
      title = "Invert";
      break;
    case BEAUTIFY_EFFECT_CLASSIC_LOMO:
      title = "Classic LOMO";
      break;
    case BEAUTIFY_EFFECT_RETRO_LOMO:
      title = "Retro LOMO";
      break;
    case BEAUTIFY_EFFECT_GOTHIC_STYLE:
      title = "Gothic Style";
      break;
    case BEAUTIFY_EFFECT_FILM:
      title = "Film";
      break;
    case BEAUTIFY_EFFECT_HDR:
      title = "HDR";
      break;
    case BEAUTIFY_EFFECT_CLASSIC_HDR:
      title = "Classic HDR";
      break;
    case BEAUTIFY_EFFECT_YELLOWING_DARK_CORNERS:
      title = "Yellow Dark\nCorner";
      break;
    case BEAUTIFY_EFFECT_IMPRESSION:
      title = "Impression";
      break;
    case BEAUTIFY_EFFECT_DEEP_BLUE_TEAR_RAIN:
      title = "Deep Blue\nTear Rain";
      break;
    case BEAUTIFY_EFFECT_PURPLE_SENSATION:
      title = "Purple\nSensation";
      break;
    case BEAUTIFY_EFFECT_BRONZE:
      title = "Bronze";
      break;
    case BEAUTIFY_EFFECT_RECALL:
      title = "Recall";
      break;
    case BEAUTIFY_EFFECT_ELEGANT:
      title = "Elegant";
      break;
    case BEAUTIFY_EFFECT_LITTLE_FRESH:
      title = "Little Fresh";
      break;
    case BEAUTIFY_EFFECT_CLASSIC_STUDIO:
      title = "Classic Studio";
      break;
    case BEAUTIFY_EFFECT_RETRO:
      title = "Retro";
      break;
    case BEAUTIFY_EFFECT_PINK_LADY:
      title = "Pink Lady";
      break;
    case BEAUTIFY_EFFECT_ABAO_COLOR:
      title = "ABao Color";
      break;
    case BEAUTIFY_EFFECT_ICE_SPIRIT:
      title = "Ice Spirit";
      break;
    case BEAUTIFY_EFFECT_JAPANESE_STYLE:
      title = "Japanese";
      break;
    case BEAUTIFY_EFFECT_NEW_JAPANESE_STYLE:
      title = "New\nJapanese";
      break;
    case BEAUTIFY_EFFECT_MILK:
      title = "Milk";
      break;
    case BEAUTIFY_EFFECT_OLD_PHOTOS:
      title = "Old Photos";
      break;
    case BEAUTIFY_EFFECT_WARM_YELLOW:
      title = "Warm Yellow";
      break;
    case BEAUTIFY_EFFECT_BLUES:
      title = "Blues";
      break;
    case BEAUTIFY_EFFECT_COLD_BLUE:
      title = "Cold Blue";
      break;
    case BEAUTIFY_EFFECT_COLD_GREEN:
      title = "Cold Green";
      break;
    case BEAUTIFY_EFFECT_PURPLE_FANTASY:
      title = "Purple\nFantasy";
      break;
    case BEAUTIFY_EFFECT_COLD_PURPLE:
      title = "Cold Purple";
      break;
    case BEAUTIFY_EFFECT_BRIGHT_RED:
      title = "Bright Red";
      break;
    case BEAUTIFY_EFFECT_CHRISTMAS_EVE:
      title = "Christmas Eve";
      break;
    case BEAUTIFY_EFFECT_NIGHT_VIEW:
      title = "Night View";
      break;
    case BEAUTIFY_EFFECT_ASTRAL:
      title = "Astral";
      break;
    case BEAUTIFY_EFFECT_COLORFUL_GLOW:
      title = "Colorful Glow";
      break;
    case BEAUTIFY_EFFECT_PICK_LIGHT:
      title = "Pick Light";
      break;
    case BEAUTIFY_EFFECT_GLASS_DROPS:
      title = "Glass Drops";
      break;
    case BEAUTIFY_EFFECT_SKETCH:
      title = "Sketch";
      break;
    case BEAUTIFY_EFFECT_LIFE_SKETCH:
      title = "Life Sketch";
      break;
    case BEAUTIFY_EFFECT_CLASSIC_SKETCH:
      title = "Classic Sketch";
      break;
    case BEAUTIFY_EFFECT_COLOR_PENCIL:
      title = "Color Pencil";
      break;
    case BEAUTIFY_EFFECT_TV_LINES:
      title = "TV Lines";
      break;
    case BEAUTIFY_EFFECT_RELIEF:
      title = "Relief";
      break;
    case BEAUTIFY_EFFECT_BEAM_GRADIENT:
      title = "Beam\nGradient";
      break;
    case BEAUTIFY_EFFECT_SUNSET_GRADIENT:
      title = "Sunset\nGradient";
      break;
    case BEAUTIFY_EFFECT_RAINBOW_GRADIENT:
      title = "Rainbow\nGradient";
      break;
    case BEAUTIFY_EFFECT_PINK_PURPLE_GRADIENG:
      title = "Pink Purple\nGradient";
      break;
    case BEAUTIFY_EFFECT_PINK_BLUE_GRADIENT:
      title = "Pink Blue\nGradient";
      break;
  }

  gint32 image = gimp_image_duplicate (thumbnail);

  run_effect (image, effect);

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  GdkPixbuf *pixbuf = gimp_image_get_thumbnail (image, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GIMP_PIXBUF_SMALL_CHECKS);
  GtkWidget *icon = gtk_image_new_from_pixbuf (pixbuf);
  GtkWidget *event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (event_box), icon);
  gtk_widget_show (icon);
  gtk_box_pack_start (GTK_BOX (box), event_box, FALSE, FALSE, 0);
  gtk_widget_show (event_box);

  GtkWidget *label = gtk_label_new (title);
  /* gtk_label_set_line_wrap will let textalign to left instead of center :(
  gtk_widget_set_size_request (label, THUMBNAIL_SIZE, -1);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);*/
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  g_signal_connect (event_box, "button_press_event", G_CALLBACK (select_effect), (gpointer) effect);

  gimp_image_delete (image);

  return box;
}

static gboolean
select_effect (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  apply_effect();
  reset_adjustment ();


  BeautifyEffectType effect = (BeautifyEffectType) user_data;
  run_effect (preview_image, effect);
  current_effect = effect;
  preview_update (preview);

  /* effect option */
  gtk_range_set_value (GTK_RANGE (effect_opacity), 100);
  gtk_widget_show (effect_option);

  return TRUE;
}

static void
reset_adjustment ()
{
  if (bvals.brightness != 0) {
    bvals.brightness = 0;
    gtk_range_set_value (GTK_RANGE (brightness), 0);
  }
  if (bvals.contrast != 0) {
    bvals.contrast = 0;
    gtk_range_set_value (GTK_RANGE (contrast), 0);
  }
  if (bvals.saturation != 0) {
    bvals.saturation = 0;
    gtk_range_set_value (GTK_RANGE (saturation), 0);
  }
  if (bvals.definition != 0) {
    bvals.definition = 0;
    gtk_range_set_value (GTK_RANGE (definition), 0);
  }
  if (bvals.hue != 0) {
    bvals.hue = 0;
    gtk_range_set_value (GTK_RANGE (hue), 0);
  }
  if (bvals.cyan_red != 0) {
    bvals.cyan_red = 0;
    gtk_range_set_value (GTK_RANGE (cyan_red), 0);
  }
  if (bvals.magenta_green != 0) {
    bvals.magenta_green = 0;
    gtk_range_set_value (GTK_RANGE (magenta_green), 0);
  }
  if (bvals.yellow_blue != 0) {
    bvals.yellow_blue = 0;
    gtk_range_set_value (GTK_RANGE (yellow_blue), 0);
  }
}

static void
apply_effect ()
{
  gtk_widget_hide (effect_option);
  /* apply effect to real image */
  if (current_effect != BEAUTIFY_EFFECT_NONE) {
    run_effect(real_image, current_effect);
    /* update opacity */
    gdouble opacity = gtk_range_get_value (GTK_RANGE (effect_opacity));
    if (opacity < 100) {
      gint32 layer = gimp_image_get_active_layer (real_image);
      g_print("Apply Effect\n=================\nActive Layer: %i\n", layer);
      gimp_layer_set_opacity (layer, opacity);
    }
  }
  adjustment(real_image);

  /* del saved_image if necessary */
  if (saved_image) {
    gimp_image_delete (saved_image);
    saved_image = 0;
  }
}

static void
cancel_effect ()
{
  if (current_effect == BEAUTIFY_EFFECT_NONE) {
    return;
  }

  gint32 current_layer = gimp_image_get_active_layer (preview_image);
  gimp_drawable_delete (current_layer);

  current_effect = BEAUTIFY_EFFECT_NONE;

  gtk_widget_hide (effect_option);

  if (saved_image) {
    gimp_image_delete (saved_image);
    saved_image = 0;
  }
}
