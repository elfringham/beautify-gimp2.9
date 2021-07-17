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

/* fix gimp_context_set_pattern ("Clipboard") only works on English versions of Gimp */
#include <glib/gi18n.h>
#define GETTEXT_PACKAGE "gimp20"
#define INIT_I18N()	G_STMT_START{                                \
  bindtextdomain (GETTEXT_PACKAGE,                    \
                  gimp_locale_directory ());                         \
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8"); \
  textdomain (GETTEXT_PACKAGE);		             \
}G_STMT_END

#include "simple-border-textures.h"

#define PLUG_IN_PROC   "plug-in-simple-border"
#define PLUG_IN_BINARY "border"
#define PLUG_IN_ROLE   "gimp-border"

#define TEXTURE_PATH     "simple-border"

#define PREVIEW_SIZE  480
#define THUMBNAIL_SIZE  80

typedef struct
{
  const gchar  *id;
  gint32        top;
  gint32        bottom;
  gint32        left;
  gint32        right;
  gint32        length;
  const guint8 *texture;
} Border;

typedef struct
{
  const Border* border;
  const gchar *custom_texture;
} BorderValues;

static void     query    (void);
static void     run      (const gchar      *name,
                          gint              nparams,
                          const GimpParam  *param,
                          gint             *nreturn_vals,
                          GimpParam       **return_vals);

static void     border (gint32     image_ID);

static gboolean border_dialog (gint32 image_ID,
                               GimpDrawable *drawable);

static void     create_texture_page (GtkNotebook *notebook, const gchar *category, const Border* textures, guint n_textures);
static gboolean create_custom_texture_pages (GtkNotebook *notebook, const gchar *texture_path);
static void     create_custom_texture_page (GtkNotebook *notebook, const gchar *category, const gchar *path);
static void     textures_switch_page (GtkNotebook *notebook, GtkWidget *page, guint page_num, const gchar *texture_path);
static GtkWidget* effect_icon_new (const Border *border);

static gboolean texture_press (GtkWidget *event_box, GdkEventButton *event, const Border *border);
static gboolean custom_texture_press (GtkWidget *event_box, GdkEventButton *event, const gchar *texture);
static void     do_texture_press ();

static inline gboolean
is_hidden (const gchar *filename)
{
  /* skip files starting with '.' so we don't try to parse
   * stuff like .DS_Store or other metadata storage files
   */
  return (filename[0] == '.');
}

const GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static BorderValues bvals =
{
  NULL,                 /* border */
};

static gint32     image_ID         = 0;
static gint       width;
static gint       height;

static GtkWidget *preview          = NULL;
static gint32     preview_image    = 0;

static const Border textures[] =
{
  {"15356",   0, 0, 0, 0, 10, texture_15356},
  {"15327",   24,37,29,41,10, texture_15327},
  {"201544",  12,12,12,12,10, texture_201544},
  {"15352",   9, 35,7, 6, 10, texture_15352},
  {"201365",  24,23,23,23,10, texture_201365},
  {"15349",   56,63,54,56,10, texture_15349},
  {"24252",   40,43,35,38,90, texture_24252},
  {"114844",  14,13,14,14,10, texture_114844},
  {"200832",  14,14,14,14,10, texture_200832},
  {"200878",  12,12,12,12,10, texture_200878},
  {"201017",  0, 0, 0, 0, 10, texture_201017},
  {"114568",  0, 0, 0, 0, 10, texture_114568},
  {"111026",  0, 0, 0, 0, 10, texture_111026},
  {"200377",  20,166,20,20,10,texture_200377},
  {"201385",  11,12,12,12,10, texture_201385},
  {"111025",  25,56,16,16,10, texture_111025},
  {"200547",  21,21,17,17,10, texture_200547},
  {"200776",  16,16,16,16,10, texture_200776},
  {"200646",  18,18, 8, 8,18, texture_200646},
  {"200282",  0, 0, 0, 0, 16, texture_200282},
  {"111020",  16,18,17,19,10, texture_111020},
  {"113579",  28,28,25,25,42, texture_113579},
  {"111569",  0, 0, 0, 0, 10, texture_111569},
  {"200031",  0, 0, 0, 0, 10, texture_200031},
  {"113576",  39,39,40,40,36, texture_113576},
  {"114907",  0, 0, 0, 0, 16, texture_114907},
  {"114577",  0, 0, 0, 0, 44, texture_114577},
  {"113575",  38,38,38,38,34, texture_113575},
  {"113573",  27,22,26,24,10, texture_113573},
  {"114576",  0, 0, 0, 0, 10, texture_114576},
  {"111013",  35,35,35,35,10, texture_111013},
  {"114575",  0, 0, 0, 0, 10, texture_114575},
  {"111017",  31,31,31,31,10, texture_111017},
  {"114573",  10,10,10,10,10, texture_114573},
  {"111027",  17,16,16,19,10, texture_111027},
  {"111021",  18,18,21,22,22, texture_111021},
  {"114572",  16,16,14,14,10, texture_114572},
  {"114570",  20,22,21,19,21, texture_114570},
  {"22753",   34,36,34,36,53, texture_22753},
  {"20130",   59,59,0, 0, 10, texture_20130},
  {"114569",  20,20,20,19,21, texture_114569},
  {"20129",   83,82,0, 0, 10, texture_20129},
  {"15354",   30,45,47,58,10, texture_15354},
  {"15357",   45,45,0, 0, 32, texture_15357},
  {"15353",   13,48,10,11,10, texture_15353},
  {"15351",   15,73,16,17,10, texture_15351},
  {"15328",   7, 7, 7, 7, 16, texture_15328},
  {"15350",   0, 0, 0, 0, 10, texture_15350},
  {"15329",   13,14,20,19,10, texture_15329},
  {"15330",   26,23,25,27,10, texture_15330},
  {"114567",  14,13,13,13,10, texture_114567},
  {"15870",   39,50,41,192,63,texture_15870},
  {"21926",   43,53,43,212,429,texture_21926},
  {"21959",   42,50,44,216,74,texture_21959},
  {"21927",   43,53,43,212,23,texture_21927},
  {"15871",   39,50,41,210,16,texture_15871},
};

static GArray *textures_timestamps = NULL;

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
  };

  gimp_install_procedure (PLUG_IN_PROC,
                          "Create an simple border effect",
                          "Create an simple border effect",
                          "Hejian <hejian.he@gmail.com>",
                          "Hejian <hejian.he@gmail.com>",
                          "2012",
                          "_Simple Border...",
                          "RGB*, GRAY*",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  gimp_plugin_menu_register (PLUG_IN_PROC,     "<Image>/Filters/Beautify/Border");
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[2];
  GimpDrawable      *drawable; //Was GimpDrawable converted to GeglBufer
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
        if (! border_dialog (image_ID, drawable))
          return;
        break;

      case GIMP_RUN_NONINTERACTIVE:
        break;

      case GIMP_RUN_WITH_LAST_VALS:
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
      border (image_ID);
      gimp_image_undo_group_end (image_ID);

      /* If run mode is interactive, flush displays */
      if (run_mode != GIMP_RUN_NONINTERACTIVE)
        gimp_displays_flush ();

      /* Store data */
      /*if (run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PLUG_IN_PROC, &rbvals, sizeof (BorderValues));*/

    }

  gimp_drawable_detach (drawable);
}

static void
border (gint32 image_ID)
{
  GdkPixbuf  *pixbuf = NULL;
  int         texture_width;
  int         texture_height;
  gdouble     margin_top;
  gdouble     margin_bottom;
  gdouble     margin_left;
  gdouble     margin_right;
  gint32      feather;
  gdouble     feather_radius_x;
  gdouble     feather_radius_y;

  pixbuf = gdk_pixbuf_new_from_inline (-1, bvals.border->texture, FALSE, NULL);

  if (pixbuf)
  {
    texture_width = gdk_pixbuf_get_width (pixbuf);
    texture_height = gdk_pixbuf_get_height (pixbuf);

    gint32 texture_image = gimp_image_new (texture_width, texture_height, GIMP_RGB);
    gint32 texture_layer = gimp_layer_new_from_pixbuf (texture_image, "texture", pixbuf, 100, GIMP_NORMAL_MODE, 0, 0);
    gimp_image_add_layer (texture_image, texture_layer, -1);

    gint width = gimp_image_width (image_ID);
    gint height = gimp_image_height (image_ID);

    if (bvals.border->top || bvals.border->bottom || bvals.border->left || bvals.border->right)
    {
      width += bvals.border->left + bvals.border->right;
      height += bvals.border->top + bvals.border->bottom;
      gimp_image_resize (image_ID,
                         width,
                         height,
                         bvals.border->left,
                         bvals.border->top);
    }

    gint32 layer = gimp_layer_new (image_ID, "border",
                                   width, height,
                                   GIMP_RGBA_IMAGE,
                                   100,
                                   GIMP_NORMAL_MODE);
    gimp_image_add_layer (image_ID, layer, -1);

    if (width > texture_width - bvals.border->length)
    {
      margin_left = ((gdouble) texture_width - bvals.border->left - bvals.border->right - bvals.border->length) / 2;
      margin_right = margin_left;
    }
    else
    {
      margin_left = ((gdouble) width - bvals.border->left - bvals.border->right) / 2;
      margin_right = margin_left;
    }
    margin_left += bvals.border->left;
    margin_right += bvals.border->right;

    if (height > texture_height - bvals.border->length)
    {
      margin_top = ((gdouble) texture_height - bvals.border->top - bvals.border->bottom - bvals.border->length) / 2;
      margin_bottom = margin_top;
    }
    else
    {
      margin_top = ((gdouble) height - bvals.border->top - bvals.border->bottom) / 2;
      margin_bottom = margin_top;
    }
    margin_top += bvals.border->top;
    margin_bottom += bvals.border->bottom;

    /* fix gimp_context_set_pattern ("Clipboard") only works on English versions of Gimp */
    //gimp_context_set_pattern ("Clipboard");
    INIT_I18N ();
    gimp_context_set_pattern (_("Clipboard"));

    feather = gimp_context_get_feather();
    gimp_context_get_feather_radius(&feather_radius_x, &feather_radius_y);
    gimp_context_set_feather(FALSE);
    gimp_context_set_feather_radius(0.0, 0.0);

    if (width > margin_left + margin_right) {
      /* top */
      gimp_image_select_rectangle (texture_image,
                        GIMP_CHANNEL_OP_REPLACE,
                        margin_left,
                        0,
                        (gdouble) texture_width - margin_left - margin_right,
                        margin_top);
      gimp_edit_copy (texture_layer);
      gimp_image_select_rectangle (image_ID,
                        GIMP_CHANNEL_OP_REPLACE,
                        margin_left,
                        0,
                        (gdouble) width - margin_left - margin_right,
                        margin_top);
      gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);

      /* bottom */
      gimp_image_select_rectangle (texture_image,
                        GIMP_CHANNEL_OP_REPLACE,
                        margin_left,
                        (gdouble) texture_height - margin_bottom,
                        (gdouble) texture_width - margin_left - margin_right,
                        margin_bottom);
      gimp_edit_copy (texture_layer);
      gimp_image_select_rectangle (image_ID,
                        GIMP_CHANNEL_OP_REPLACE,
                        margin_left,
                        (gdouble) height - margin_bottom,
                        (gdouble) width - margin_left - margin_right,
                        margin_bottom);
      gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);
    }

    if (height > margin_top + margin_bottom) {
      /* left */
      gimp_image_select_rectangle (texture_image,
                        GIMP_CHANNEL_OP_REPLACE,
                        0,
                        margin_top,
                        margin_left,
                        (gdouble) texture_height - margin_top - margin_bottom);
      gimp_edit_copy (texture_layer);
      gimp_image_select_rectangle (image_ID,
                        GIMP_CHANNEL_OP_REPLACE,
                        0,
                        margin_top,
                        margin_left,
                        (gdouble) height - margin_top - margin_bottom);
      gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);

      /* right */
      gimp_image_select_rectangle (texture_image,
                        GIMP_CHANNEL_OP_REPLACE,
                        (gdouble) texture_width - margin_right,
                        margin_top,
                        margin_right,
                        (gdouble) texture_height - margin_top - margin_bottom);
      gimp_edit_copy (texture_layer);
      gimp_image_select_rectangle (image_ID,
                        GIMP_CHANNEL_OP_REPLACE,
                        (gdouble) width - margin_right,
                        margin_top,
                        margin_right,
                        (gdouble) height - margin_top - margin_bottom);
      gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);
    }

    /* top left */
    gimp_image_select_rectangle (texture_image,
                      GIMP_CHANNEL_OP_REPLACE,
                      0,
                      0,
                      margin_left,
                      margin_top);
    gimp_edit_copy (texture_layer);
    gimp_image_select_rectangle (image_ID,
                      GIMP_CHANNEL_OP_REPLACE,
                      0,
                      0,
                      margin_left,
                      margin_top);
    gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);

    /* top right */
    gimp_image_select_rectangle (texture_image,
                      GIMP_CHANNEL_OP_REPLACE,
                      (gdouble) texture_width - margin_right,
                      0,
                      margin_right,
                      margin_top);
    gimp_edit_copy (texture_layer);
    gimp_image_select_rectangle (image_ID,
                      GIMP_CHANNEL_OP_REPLACE,
                      (gdouble) width - margin_right,
                      0,
                      margin_right,
                      margin_top);
    gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);

    /* bottom left */
    gimp_image_select_rectangle (texture_image,
                      GIMP_CHANNEL_OP_REPLACE,
                      0,
                      (gdouble) texture_height - margin_bottom,
                      margin_left,
                      margin_bottom);
    gimp_edit_copy (texture_layer);
    gimp_image_select_rectangle (image_ID,
                      GIMP_CHANNEL_OP_REPLACE,
                      0,
                      (gdouble) height - margin_bottom,
                      margin_left,
                      margin_bottom);
    gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);

    /* bottom right */
    gimp_image_select_rectangle (texture_image,
                      GIMP_CHANNEL_OP_REPLACE,
                      (gdouble) texture_width - margin_right,
                      (gdouble) texture_height - margin_bottom,
                      margin_right,
                      margin_bottom);
    gimp_edit_copy (texture_layer);
    gimp_image_select_rectangle (image_ID,
                      GIMP_CHANNEL_OP_REPLACE,
                      (gdouble) width - margin_right,
                      (gdouble) height - margin_bottom,
                      margin_right,
                      margin_bottom);
    gimp_drawable_edit_fill (layer, GIMP_PATTERN_FILL);

    gimp_image_merge_down(image_ID, layer, GIMP_CLIP_TO_IMAGE);

    gimp_selection_none (image_ID);

    gimp_context_set_feather(feather);
    gimp_context_set_feather_radius(feather_radius_x, feather_radius_y);
  }
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

static gboolean
border_dialog (gint32        image_ID,
               GimpDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_hbox;
  GtkWidget *middle_vbox;
  GtkWidget *right_vbox;
  GtkWidget *label;

  gboolean   run;

  gimp_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = gimp_dialog_new ("Simple Border", PLUG_IN_ROLE,
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

  middle_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (middle_vbox), 12);
  gtk_box_pack_start (GTK_BOX (main_hbox), middle_vbox, TRUE, TRUE, 0);
  gtk_widget_show (middle_vbox);

  right_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (right_vbox), 12);
  gtk_widget_set_size_request (right_vbox, 320, -1);
  gtk_box_pack_start (GTK_BOX (main_hbox), right_vbox, TRUE, TRUE, 0);
  gtk_widget_show (right_vbox);

  /* preview */
  label = gtk_label_new ("Preview");
  gtk_box_pack_start (GTK_BOX (middle_vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  preview_image = gimp_image_duplicate(image_ID);

  preview = gtk_image_new();
  preview_update (preview);

  gtk_box_pack_start (GTK_BOX (middle_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  /* textures */
  label = gtk_label_new ("Textures");
  gtk_box_pack_start (GTK_BOX (right_vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  GtkWidget *notebook = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (right_vbox), notebook, FALSE, FALSE, 0);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
  gtk_widget_show (notebook);

  create_texture_page (GTK_NOTEBOOK (notebook), "Top", textures, G_N_ELEMENTS (textures));

  run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
create_texture_page (GtkNotebook *notebook, const gchar* category, const Border* textures, guint n_textures) {
  GtkWidget *label = gtk_label_new (category);

  GtkWidget *thispage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 0);
  gtk_widget_set_size_request (thispage, -1, 480);
  gtk_widget_show (thispage);

  /* scrolled window */
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (thispage), scrolled_window, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_window);

  /* table */
  gint rows = 5;
  gint cols = 3;
  GtkWidget *table = gtk_table_new (rows, cols, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  //gtk_box_pack_start (GTK_BOX (thispage), table, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table), 10);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), table);
  gtk_widget_show (table);

  gint row = 1;
  gint col = 1;

  gint i;
  for (i = 0; i < n_textures; i++)
  {
    GtkWidget *icon = effect_icon_new (&textures[i]);
    gtk_table_attach_defaults (GTK_TABLE (table), icon, col - 1, col, row - 1, row);
    gtk_widget_show (icon);

    col++;
    if (col > cols)
    {
      row++;
      col = 1;
    }
  }

  gtk_notebook_append_page_menu (notebook, thispage, label, NULL);
}

static gboolean
create_custom_texture_pages (GtkNotebook *notebook, const gchar *texture_path)
{
  gboolean has_custom_texture = FALSE;

  const gchar *gimp_dir = gimp_directory ();
  const gchar *texture_dir = g_build_filename (gimp_dir, texture_path, NULL);
  GDir *dir = g_dir_open (texture_dir, 0, NULL);
  if (dir)
  {
    const gchar *dir_ent;
    while (dir_ent = g_dir_read_name (dir))
    {
      if (is_hidden (dir_ent))
        continue;

      gchar *filename = g_build_filename (texture_dir, dir_ent, NULL);
      if (g_file_test (filename, G_FILE_TEST_IS_DIR)) {
        create_custom_texture_page (GTK_NOTEBOOK (notebook), dir_ent, filename);
        has_custom_texture = TRUE;
      }
    }
  }

  return has_custom_texture;
}

static void
create_custom_texture_page (GtkNotebook *notebook, const gchar* category, const gchar* path) {
  GtkWidget *label = gtk_label_new (category);

  GtkWidget *thispage = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 12);
  gtk_widget_show (thispage);

  gtk_notebook_append_page_menu (notebook, thispage, label, NULL);
}

static GtkWidget *
effect_icon_new (const Border *border)
{
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  /* image */
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_inline (-1, border->texture, FALSE, NULL);
  pixbuf = gdk_pixbuf_scale_simple (pixbuf, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GDK_INTERP_BILINEAR);
  GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
  GtkWidget *event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (event_box), image);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (box), event_box, FALSE, FALSE, 0);
  gtk_widget_show (event_box);

  g_signal_connect (event_box, "button_press_event", G_CALLBACK (texture_press), (gpointer)border);

  /* label */
  GtkWidget *label = gtk_label_new (border->id);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  return box;
}

static void
textures_switch_page (GtkNotebook *notebook, GtkWidget *page, guint page_num, const gchar *texture_path)
{
  if (page_num == 0 || g_array_index (textures_timestamps, time_t, page_num) > 0)
    return;

  // fix gtk2
  page = gtk_notebook_get_nth_page (notebook, page_num);

  gtk_container_set_border_width (GTK_CONTAINER (page), 0);
  gtk_widget_set_size_request (page, -1, 480);

  const gchar *category = gtk_notebook_get_tab_label_text(notebook, page);

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

  const gchar *gimp_dir = gimp_directory ();
  const gchar *texture_dir = g_build_filename (gimp_dir, texture_path, NULL);
  const gchar *path = g_build_filename (texture_dir, category, NULL);

  GDir *dir = g_dir_open (path, 0, NULL);
  if (dir)
  {
    const gchar *dir_ent;
    while (dir_ent = g_dir_read_name (dir))
    {
      if (is_hidden (dir_ent))
        continue;

      gchar *filename = g_build_filename (path, dir_ent, NULL);
      GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
      pixbuf = gdk_pixbuf_scale_simple (pixbuf, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GDK_INTERP_BILINEAR);
      GtkWidget *image = gtk_image_new_from_pixbuf (pixbuf);
      GtkWidget *event_box = gtk_event_box_new ();
      gtk_container_add (GTK_CONTAINER (event_box), image);
      gtk_widget_show (image);
      gtk_table_attach_defaults (GTK_TABLE (table), event_box, col - 1, col, row - 1, row);
      gtk_widget_show (event_box);

      col++;
      if (col > cols)
      {
        row++;
        col = 1;
      }

      g_signal_connect (event_box, "button_press_event", G_CALLBACK (custom_texture_press), filename);
    }
  }

  g_array_index (textures_timestamps, time_t, page_num) = time (NULL);
}

static gboolean
texture_press (GtkWidget *event_box, GdkEventButton *event, const Border* texture)
{
  bvals.border = texture;
  bvals.custom_texture = NULL;

  do_texture_press ();
}

static gboolean
custom_texture_press (GtkWidget *event_box, GdkEventButton *event, const gchar *texture)
{
  bvals.border = NULL;
  bvals.custom_texture = texture;

  do_texture_press ();
}

static void
do_texture_press ()
{
  gimp_image_delete (preview_image);
  preview_image = gimp_image_duplicate(image_ID);

  border (preview_image);

  preview_update (preview);
}
