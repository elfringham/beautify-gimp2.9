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

// fix gimp-2.6 issue: Procedure 'gimp-layer-new' has been called with value '100' for argument 'mode'
#include <libgimp/gimpui.h>

#include "beautify-effect.h"
#include "beautify-textures.h"

static void black_and_white (gint32 image_ID, gint32 drawable_ID)
{
  gint       nreturn_vals;
  GimpParam *return_vals;

  return_vals = gimp_run_procedure ("plug-in-colors-channel-mixer",
                                    &nreturn_vals,
                                    GIMP_PDB_INT32, 1,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_DRAWABLE, drawable_ID,
                                    GIMP_PDB_INT32, 0,
                                    GIMP_PDB_FLOAT, 0.30,
                                    GIMP_PDB_FLOAT, 0.59,
                                    GIMP_PDB_FLOAT, 0.11,
                                    GIMP_PDB_FLOAT, 0.30,
                                    GIMP_PDB_FLOAT, 0.59,
                                    GIMP_PDB_FLOAT, 0.11,
                                    GIMP_PDB_FLOAT, 0.30,
                                    GIMP_PDB_FLOAT, 0.59,
                                    GIMP_PDB_FLOAT, 0.11,
                                    GIMP_PDB_END);
  gimp_destroy_params (return_vals, nreturn_vals);

  //gimp_desaturate_full (drawable_ID, GIMP_DESATURATE_LUMINOSITY);
}

void
run_effect (gint32 image_ID, BeautifyEffectType effect)
{
  gimp_context_push ();

  gint32 layer = gimp_image_get_active_layer (image_ID);
  gint32 effect_layer = gimp_layer_copy (layer);
  gimp_image_insert_layer (image_ID, effect_layer, -1, -1);
  //gimp_layer_set_lock_alpha (effect_layer, TRUE);

  gint width = gimp_image_width (image_ID);
  gint height = gimp_image_height (image_ID);

  switch (effect)
  {
    case BEAUTIFY_EFFECT_SOFT_LIGHT:
    {
      gint32     layer;

      layer = gimp_layer_copy (effect_layer);
      gimp_image_insert_layer (image_ID, layer, -1, -1);
      //gimp_levels (layer, GIMP_HISTOGRAM_VALUE, 20, 255, 1, 0, 255);
      gimp_drawable_levels(layer, GIMP_HISTOGRAM_VALUE, 0.078431372549, 1.0, 1, 1.0, 0.0, 1.0, 1);

      GimpParam *return_vals;
      gint nreturn_vals;
      return_vals = gimp_run_procedure ("plug-in-gauss",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
                                        GIMP_PDB_IMAGE, image_ID,
                                        GIMP_PDB_DRAWABLE, layer,
                                        GIMP_PDB_FLOAT, 15.0,
                                        GIMP_PDB_FLOAT, 15.0,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_END);
      gimp_destroy_params (return_vals, nreturn_vals);

      gimp_layer_set_mode (layer, GIMP_SCREEN_MODE);
      gimp_layer_set_opacity (layer, 35);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_IMAGE);

      break;
    }
    case BEAUTIFY_EFFECT_WARM:
    {
      //TODO - done
      //Convert Integer math * 255 to Double Math
      gdouble red_pts[] = {
        0.0, 0.082031,
        0.405488, 0.621094,
        0.954268, 1.000000,
      };
      gdouble green_pts[] = {
        0.0, 0.0,
        0.503049, 0.636719,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 6, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 6, green_pts);
    }
      break;

    case BEAUTIFY_EFFECT_SHARPEN:
    {
      gint nreturn_vals;
      GimpParam *return_vals = gimp_run_procedure ("plug-in-sharpen",
                                                   &nreturn_vals,
                                                   GIMP_PDB_INT32, 1,
                                                   GIMP_PDB_IMAGE, image_ID,
                                                   GIMP_PDB_DRAWABLE, effect_layer,
                                                   GIMP_PDB_INT32, 50,
                                                   GIMP_PDB_END);
      gimp_destroy_params (return_vals, nreturn_vals);
      break;
    }
    case BEAUTIFY_EFFECT_SOFT:
    {
      gint nreturn_vals;
      GimpParam *return_vals = gimp_run_procedure ("plug-in-gauss",
                                                   &nreturn_vals,
                                                   GIMP_PDB_INT32, 1,
                                                   GIMP_PDB_IMAGE, image_ID,
                                                   GIMP_PDB_DRAWABLE, effect_layer,
                                                   GIMP_PDB_FLOAT, 1.2,
                                                   GIMP_PDB_FLOAT, 1.2,
                                                   GIMP_PDB_INT32, 1,
                                                   GIMP_PDB_END);
      gimp_destroy_params (return_vals, nreturn_vals);
      break;
    }
    case BEAUTIFY_EFFECT_STRONG_CONTRAST:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.039216,
        0.247059, 0.105882,
        0.372549, 0.235294,
        0.498039, 0.537255,
        0.623529, 0.768627,
        0.749020, 0.858824,
        0.874510, 0.929412,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.027451,
        0.247059, 0.117647,
        0.372549, 0.286275,
        0.498039, 0.576471,
        0.623529, 0.780392,
        0.749020, 0.890196,
        0.874510, 0.952941,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.050980,
        0.247059, 0.133333,
        0.372549, 0.294118,
        0.498039, 0.568627,
        0.623529, 0.772549,
        0.749020, 0.874510,
        0.874510, 0.941176,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_SMART_COLOR:
    {
      gdouble red_pts[] = {
        0.0, 0.001012, 0.121569, 0.126695,
        0.247059, 0.279821, 0.372549, 0.428038,
        0.498039, 0.567700, 0.623529, 0.699439,
        0.749020, 0.821423, 0.874510, 0.953474,
        1.000000, 0.997988,
      };
      gdouble green_pts[] = {
        0.0, 0.004278, 0.121569, 0.107139,
        0.247059, 0.225961, 0.372549, 0.346578,
        0.498039, 0.472647, 0.623529, 0.602136,
        0.749020, 0.730046, 0.874510, 0.873495,
        1.000000, 0.996787,
      };
      gdouble blue_pts[] = {
        0.0, 0.000105, 0.121569, 0.060601,
        0.247059, 0.146772, 0.372549, 0.262680,
        0.498039, 0.408053, 0.623529, 0.566459,
        0.749020, 0.691468, 0.874510, 0.847356,
        1.000000, 0.999226,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_BLACK_AND_WHITE:
    {
      black_and_white (image_ID, effect_layer);
      break;
    }
    case BEAUTIFY_EFFECT_INVERT:
      gimp_invert (effect_layer);
      break;
    case BEAUTIFY_EFFECT_CLASSIC_LOMO:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_classic_LOMO_1, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_OVERLAY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      gdouble red_pts[] = {
        0.000000, 0.000430,
        0.121569, 0.034226,
        0.247059, 0.156268,
        0.372549, 0.337497,
        0.498039, 0.542195,
        0.623529, 0.728355,
        0.749020, 0.862534,
        0.874510, 0.942754,
        1.000000, 0.994413,
      };
      gdouble green_pts[] = {
        0.000000, 0.000167,
        0.121569, 0.023472,
        0.247059, 0.139498,
        0.372549, 0.318074,
        0.498039, 0.520901,
        0.623529, 0.705862,
        0.749020, 0.849380,
        0.874510, 0.952092,
        1.000000, 0.994484,
      };
      gdouble blue_pts[] = {
        0.000000, 0.000377,
        0.121569, 0.030137,
        0.247059, 0.160170,
        0.372549, 0.335944,
        0.498039, 0.545279,
        0.623529, 0.719690,
        0.749020, 0.867722,
        0.874510, 0.965811,
        1.000000, 0.995110,
      };
      layer = gimp_image_get_active_layer (image_ID);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_classic_LOMO_2, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_MULTIPLY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_RETRO_LOMO:
    {
      gdouble red_pts[] = {
        0.000000, 0.009477,
        0.121569, 0.066342,
        0.247059, 0.211570,
        0.372549, 0.391796,
        0.498039, 0.576389,
        0.623529, 0.745091,
        0.749020, 0.875015,
        0.874510, 0.959604,
        1.000000, 0.989234,
      };
      gdouble green_pts[] = {
        0.000000, 0.075980,
        0.121569, 0.176692,
        0.247059, 0.294329,
        0.372549, 0.415297,
        0.498039, 0.536491,
        0.623529, 0.651230,
        0.749020, 0.751355,
        0.874510, 0.843675,
        1.000000, 0.921772,
      };
      gdouble blue_pts[] = {
        0.000000, 0.246068,
        0.121569, 0.310134,
        0.247059, 0.373558,
        0.372549, 0.435251,
        0.498039, 0.503361,
        0.623529, 0.565592,
        0.749020, 0.629995,
        0.874510, 0.690267,
        1.000000, 0.751997,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint       nreturn_vals;
      GimpParam *return_vals;

      return_vals = gimp_run_procedure ("plug-in-rgb-noise",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_IMAGE, image_ID,
                                        GIMP_PDB_DRAWABLE, effect_layer,
                                        GIMP_PDB_INT32, 0,
                                        GIMP_PDB_INT32, 0,
                                        GIMP_PDB_FLOAT, 0.03,
                                        GIMP_PDB_FLOAT, 0.03,
                                        GIMP_PDB_FLOAT, 0.03,
                                        GIMP_PDB_FLOAT, 0.03,
                                        GIMP_PDB_END);
      gimp_destroy_params (return_vals, nreturn_vals);

      break;
    }
    case BEAUTIFY_EFFECT_GOTHIC_STYLE:
    {
      gdouble red_pts[] = {
        0.0, 0.003922, 0.121569, 0.011765,
        0.247059, 0.074510, 0.372549, 0.200000,
        0.498039, 0.380392, 0.623529, 0.584314,
        0.749020, 0.784314, 0.874510, 0.933333,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.0, 0.003922, 0.121569, 0.039216,
        0.247059, 0.160784, 0.372549, 0.317647,
        0.498039, 0.501961, 0.623529, 0.682353,
        0.749020, 0.843137, 0.874510, 0.952941,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.0, 0.003922, 0.121569, 0.007843,
        0.247059, 0.058824, 0.372549, 0.172549,
        0.498039, 0.349020, 0.623529, 0.556863,
        0.749020, 0.768627, 0.874510, 0.929412,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_FILM:
    {
      gdouble red_pts[] = {
        0.000000, 0.101961,
        0.121569, 0.101961,
        0.247059, 0.164706,
        0.372549, 0.333333,
        0.498039, 0.521569,
        0.623529, 0.701961,
        0.749020, 0.850980,
        0.874510, 0.956863,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.192157,
        0.121569, 0.192157,
        0.247059, 0.192157,
        0.372549, 0.329412,
        0.498039, 0.517647,
        0.623529, 0.698039,
        0.749020, 0.850980,
        0.874510, 0.956863,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.266667,
        0.121569, 0.266667,
        0.247059, 0.266667,
        0.372549, 0.290196,
        0.498039, 0.470588,
        0.623529, 0.658824,
        0.749020, 0.827451,
        0.874510, 0.949020,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_HDR:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.015686,
        0.247059, 0.207843,
        0.372549, 0.396078,
        0.498039, 0.556863,
        0.623529, 0.698039,
        0.749020, 0.811765,
        0.874510, 0.909804,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.015686,
        0.247059, 0.200000,
        0.372549, 0.384314,
        0.498039, 0.541176,
        0.623529, 0.682353,
        0.749020, 0.800000,
        0.874510, 0.901961,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.054902,
        0.121569, 0.121569,
        0.247059, 0.262745,
        0.372549, 0.407843,
        0.498039, 0.537255,
        0.623529, 0.662745,
        0.749020, 0.780392,
        0.874510, 0.890196,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_CLASSIC_HDR:
    {
      gdouble red_pts[] = {
        0.0, 0.054902, 0.121569, 0.070588,
        0.247059, 0.243137, 0.372549, 0.407843,
        0.498039, 0.552941, 0.623529, 0.678431,
        0.749020, 0.780392, 0.874510, 0.866667,
        1.000000, 0.949020,
      };
      gdouble green_pts[] = {
        0.0, 0.007843, 0.121569, 0.023529,
        0.247059, 0.207843, 0.372549, 0.388235,
        0.498039, 0.541176, 0.623529, 0.682353,
        0.749020, 0.796078, 0.874510, 0.898039,
        1.000000, 0.996078,
      };
      gdouble blue_pts[] = {
        0.0, 0.258824, 0.121569, 0.294118,
        0.247059, 0.372549, 0.372549, 0.450980,
        0.498039, 0.521569, 0.623529, 0.592157,
        0.749020, 0.654902, 0.874510, 0.717647,
        1.000000, 0.776471,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_YELLOWING_DARK_CORNERS:
    {
      gdouble red_pts[] = {
        0.000000, 0.093137,
        0.121569, 0.125134,
        0.247059, 0.227000,
        0.372549, 0.372794,
        0.498039, 0.537491,
        0.623529, 0.706434,
        0.749020, 0.852155,
        0.874510, 0.953969,
        1.000000, 0.996078,
      };
      gdouble green_pts[] = {
        0.000000, 0.092647,
        0.121569, 0.125205,
        0.247059, 0.227129,
        0.372549, 0.372871,
        0.498039, 0.537711,
        0.623529, 0.706357,
        0.749020, 0.851153,
        0.874510, 0.953240,
        1.000000, 0.996078,
      };
      gdouble blue_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.029810,
        0.247059, 0.143778,
        0.372549, 0.305764,
        0.498039, 0.488796,
        0.623529, 0.672134,
        0.749020, 0.833704,
        0.874510, 0.948010,
        1.000000, 0.996078,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_yellowing_dark_corners, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_MULTIPLY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_IMPRESSION:
    {
      gdouble red_pts[] = {
        0.000000, 0.113725,
        0.121569, 0.213975,
        0.247059, 0.323494,
        0.372549, 0.460137,
        0.498039, 0.621504,
        0.623529, 0.716847,
        0.749020, 0.766909,
        0.874510, 0.817666,
        1.000000, 0.862745,
      };
      gdouble green_pts[] = {
        0.000000, 0.200000,
        0.121569, 0.317329,
        0.247059, 0.407881,
        0.372549, 0.535429,
        0.498039, 0.682828,
        0.623529, 0.770688,
        0.749020, 0.813005,
        0.874510, 0.852891,
        1.000000, 0.902716,
      };
      gdouble blue_pts[] = {
        0.000000, 0.317714,
        0.121569, 0.364205,
        0.247059, 0.417294,
        0.372549, 0.495841,
        0.498039, 0.612710,
        0.623529, 0.719834,
        0.749020, 0.795937,
        0.874510, 0.845977,
        1.000000, 0.883024,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_DEEP_BLUE_TEAR_RAIN:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.094118,
        0.247059, 0.254902,
        0.372549, 0.439216,
        0.498039, 0.603922,
        0.623529, 0.745098,
        0.749020, 0.850980,
        0.874510, 0.941176,
        1.000000, 0.992157,
      };
      gdouble green_pts[] = {
        0.000000, 0.019608,
        0.121569, 0.164706,
        0.247059, 0.337255,
        0.372549, 0.509804,
        0.498039, 0.666667,
        0.623529, 0.784314,
        0.749020, 0.878431,
        0.874510, 0.941176,
        1.000000, 0.992157,
      };
      gdouble blue_pts[] = {
        0.000000, 0.133333,
        0.121569, 0.333333,
        0.247059, 0.494118,
        0.372549, 0.643137,
        0.498039, 0.760784,
        0.623529, 0.850980,
        0.749020, 0.913725,
        0.874510, 0.964706,
        1.000000, 0.988235,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_PURPLE_SENSATION:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.003922,
        0.247059, 0.149020,
        0.372549, 0.345098,
        0.498039, 0.541176,
        0.623529, 0.725490,
        0.749020, 0.870588,
        0.874510, 0.960784,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.003922,
        0.247059, 0.003922,
        0.372549, 0.274510,
        0.498039, 0.521569,
        0.623529, 0.713725,
        0.749020, 0.862745,
        0.874510, 0.952941,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.007843,
        0.121569, 0.070588,
        0.247059, 0.313725,
        0.372549, 0.513725,
        0.498039, 0.682353,
        0.623529, 0.811765,
        0.749020, 0.905882,
        0.874510, 0.968627,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_BRONZE:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.078431,
        0.247059, 0.196078,
        0.372549, 0.341176,
        0.498039, 0.494118,
        0.623529, 0.650980,
        0.749020, 0.729412,
        0.874510, 0.729412,
        1.000000, 0.729412,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.078431,
        0.247059, 0.196078,
        0.372549, 0.341176,
        0.498039, 0.494118,
        0.623529, 0.650980,
        0.749020, 0.792157,
        0.874510, 0.913725,
        1.000000, 0.925490,
      };
      gdouble blue_pts[] = {
        0.000000, 0.450980,
        0.121569, 0.450980,
        0.247059, 0.450980,
        0.372549, 0.450980,
        0.498039, 0.494118,
        0.623529, 0.650980,
        0.749020, 0.792157,
        0.874510, 0.913725,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_RECALL:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_recall, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_MULTIPLY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      g_print("Beautify Effect Recall\n Active Layer: %i Image ID: %i\n", layer, image_ID);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);
      break;
    }
    case BEAUTIFY_EFFECT_ELEGANT:
    {
      gimp_drawable_hue_saturation (effect_layer, GIMP_ALL_HUES, 0.0, 0.0, -40, 0.0); // new extra overlap arguments gimp_drawable_hue_saturation (gint32 drawable_ID, GimpHueRange hue_range, gdouble hue_offset, gdouble lightness, gdouble saturation, gdouble overlap);

      gdouble red_pts[] = {
        0.000000, 0.000171,
        0.121569, 0.047560,
        0.247059, 0.162212,
        0.372549, 0.329729,
        0.498039, 0.520206,
        0.623529, 0.704597,
        0.749020, 0.849098,
        0.874510, 0.971076,
        1.000000, 0.996078,
      };
      gdouble green_pts[] = {
        0.000000, 0.007843,
        0.121569, 0.063296,
        0.247059, 0.177207,
        0.372549, 0.326921,
        0.498039, 0.494137,
        0.623529, 0.653138,
        0.749020, 0.799763,
        0.874510, 0.903914,
        1.000000, 0.949020,
      };
      gdouble blue_pts[] = {
        0.000000, 0.031373,
        0.121569, 0.103265,
        0.247059, 0.214688,
        0.372549, 0.347533,
        0.498039, 0.485165,
        0.623529, 0.619805,
        0.749020, 0.751388,
        0.874510, 0.868222,
        1.000000, 0.949020,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      break;
    }
    case BEAUTIFY_EFFECT_LITTLE_FRESH:
    {
      gdouble red_pts[] = {
        0.0, 0.002975, 0.121569, 0.135413,
        0.247059, 0.271797, 0.372549, 0.420642,
        0.498039, 0.587088, 0.623529, 0.672206,
        0.749020, 0.781208, 0.874510, 0.881668,
        1.000000, 0.993149,
      };
      gdouble green_pts[] = {
        0.0, 0.001070, 0.121569, 0.123393,
        0.247059, 0.254300, 0.372549, 0.377336,
        0.498039, 0.486582, 0.623529, 0.607331,
        0.749020, 0.722174, 0.874510, 0.858206,
        1.000000, 0.992154,
      };
      gdouble blue_pts[] = {
        0.0, 0.003917, 0.121569, 0.098807,
        0.247059, 0.234746, 0.372549, 0.378388,
        0.498039, 0.520273, 0.623529, 0.633239,
        0.749020, 0.748242, 0.874510, 0.862234,
        1.000000, 0.964176,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_CLASSIC_STUDIO:
    {
      gdouble red_pts[] = {
        0.000000, 0.002941,
        0.121569, 0.105177,
        0.247059, 0.276869,
        0.372549, 0.449951,
        0.498039, 0.615011,
        0.623529, 0.765528,
        0.749020, 0.884498,
        0.874510, 0.964439,
        1.000000, 0.996641,
      };
      gdouble green_pts[] = {
        0.000000, 0.000980,
        0.121569, 0.023976,
        0.247059, 0.117564,
        0.372549, 0.268570,
        0.498039, 0.450785,
        0.623529, 0.640827,
        0.749020, 0.821280,
        0.874510, 0.944143,
        1.000000, 0.994046,
      };
      gdouble blue_pts[] = {
        0.000000, 0.001705,
        0.121569, 0.091176,
        0.247059, 0.255272,
        0.372549, 0.426934,
        0.498039, 0.599930,
        0.623529, 0.749604,
        0.749020, 0.879809,
        0.874510, 0.963030,
        1.000000, 0.994565,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_RETRO:
    {
      gdouble red_pts[] = {
        0.000000, 0.011765,
        0.121569, 0.050275,
        0.247059, 0.163976,
        0.372549, 0.316983,
        0.498039, 0.493141,
        0.623529, 0.671170,
        0.749020, 0.829955,
        0.874510, 0.941938,
        1.000000, 0.988797,
      };
      gdouble green_pts[] = {
        0.000000, 0.044118,
        0.121569, 0.081048,
        0.247059, 0.181188,
        0.372549, 0.327417,
        0.498039, 0.493717,
        0.623529, 0.658936,
        0.749020, 0.811563,
        0.874510, 0.915557,
        1.000000, 0.956299,
      };
      gdouble blue_pts[] = {
        0.000000, 0.247630,
        0.121569, 0.268491,
        0.247059, 0.325230,
        0.372549, 0.405204,
        0.498039, 0.497829,
        0.623529, 0.588839,
        0.749020, 0.675181,
        0.874510, 0.731610,
        1.000000, 0.752075,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_PINK_LADY:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.196078,
        0.247059, 0.356863,
        0.372549, 0.509804,
        0.498039, 0.647059,
        0.623529, 0.760784,
        0.749020, 0.858824,
        0.874510, 0.937255,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.180392,
        0.247059, 0.329412,
        0.372549, 0.478431,
        0.498039, 0.611765,
        0.623529, 0.729412,
        0.749020, 0.831373,
        0.874510, 0.921569,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.168627,
        0.247059, 0.317647,
        0.372549, 0.458824,
        0.498039, 0.592157,
        0.623529, 0.709804,
        0.749020, 0.819608,
        0.874510, 0.913725,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_ABAO_COLOR:
    {
      gint       nreturn_vals;
      GimpParam *return_vals;
      gint      *layers;
      gint       num_layers;
      gint32     layer;
      gint32     image;

      return_vals = gimp_run_procedure ("plug-in-decompose",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_IMAGE, image_ID,
                                        GIMP_PDB_DRAWABLE, effect_layer,
                                        GIMP_PDB_STRING, "LAB",
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_END);
      gint32 lab = return_vals[1].data.d_int32;
      gimp_destroy_params (return_vals, nreturn_vals);

      layers = gimp_image_get_layers (lab, &num_layers);
      return_vals = gimp_run_procedure ("plug-in-drawable-compose",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_IMAGE, lab,
                                        GIMP_PDB_DRAWABLE, layers[0],
                                        GIMP_PDB_DRAWABLE, layers[1],
                                        GIMP_PDB_DRAWABLE, layers[2],
                                        GIMP_PDB_DRAWABLE, 0,
                                        GIMP_PDB_STRING, "RGB",
                                        GIMP_PDB_END);
      gimp_image_delete (lab);
      image = return_vals[1].data.d_int32;
      gimp_destroy_params (return_vals, nreturn_vals);

      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.132068,
        0.247059, 0.270845,
        0.372549, 0.400251,
        0.498039, 0.526639,
        0.623529, 0.644370,
        0.749020, 0.777614,
        0.874510, 0.890458,
        1.000000, 0.998224,
      };
      gdouble green_pts[] = {
        0.000000, 0.247059,
        0.121569, 0.258731,
        0.247059, 0.288818,
        0.372549, 0.362942,
        0.498039, 0.494041,
        0.623529, 0.613636,
        0.749020, 0.736961,
        0.874510, 0.860510,
        1.000000, 0.905882,
      };
      gdouble blue_pts[] = {
        0.000000, 0.766094,
        0.121569, 0.762986,
        0.247059, 0.708980,
        0.372549, 0.587155,
        0.498039, 0.507613,
        0.623529, 0.478932,
        0.749020, 0.458030,
        0.874510, 0.434199,
        1.000000, 0.454517,
      };
      layer = gimp_image_get_active_layer (image);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      layer = gimp_image_get_active_layer (image);
      return_vals = gimp_run_procedure ("plug-in-decompose",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_IMAGE, image,
                                        GIMP_PDB_DRAWABLE, layer,
                                        GIMP_PDB_STRING, "RGB",
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_END);
      gimp_image_delete (image);
      gint32 rgb = return_vals[1].data.d_int32;
      gimp_destroy_params (return_vals, nreturn_vals);

      layers = gimp_image_get_layers (rgb, &num_layers);
      return_vals = gimp_run_procedure ("plug-in-drawable-compose",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_IMAGE, rgb,
                                        GIMP_PDB_DRAWABLE, layers[0],
                                        GIMP_PDB_DRAWABLE, layers[1],
                                        GIMP_PDB_DRAWABLE, layers[2],
                                        GIMP_PDB_DRAWABLE, 0,
                                        GIMP_PDB_STRING, "LAB",
                                        GIMP_PDB_END);
      gimp_image_delete (rgb);
      image = return_vals[1].data.d_int32;
      gimp_destroy_params (return_vals, nreturn_vals);

      layer = gimp_image_get_active_layer (image);
      gimp_edit_copy (layer);
      gint32 floating_sel = gimp_edit_paste (effect_layer, FALSE);
      gimp_floating_sel_anchor (floating_sel);
      g_print("EFFECT_ABAO_COLOR Floating Selection: %i\n", floating_sel);

      gimp_image_delete (image);

      /*GimpPixelRgn src_rgn, dest_rgn;
      gint x1, y1;
      gint width, height;
      gpointer pr;

      GimpDrawable *drawable = gimp_drawable_get (effect_layer);
      if (!gimp_drawable_mask_intersect (drawable->drawable_id,
          &x1, &y1, &width, &height))
        return;

      gimp_pixel_rgn_init (&src_rgn, drawable, x1, y1, width, height, FALSE, FALSE);
      gimp_pixel_rgn_init (&dest_rgn, drawable, x1, y1, width, height, TRUE, TRUE);
      for (pr = gimp_pixel_rgns_register (2, &src_rgn, &dest_rgn);
           pr != NULL;
           pr = gimp_pixel_rgns_process (pr))
      {
        const guchar *src = src_rgn.data;
        guchar *dest = dest_rgn.data;
        gint x, y;

        for (y = 0; y < src_rgn.h; y++)
        {
          const guchar *s = src;
          guchar *d = dest;

          for (x = 0; x < src_rgn.w; x++)
          {
            gdouble r, g, b;
            gdouble v;

            r = (gdouble) s[0] / 255;
            g = (gdouble) s[1] / 255;
            b = (gdouble) s[2] / 255;

            v = b - 154.0 / 255;
            v = MAX (0, v);
            v = r + v - g * 172.0 / 255;
            v = MIN (1, MAX (0, v));
            v = sin(v * G_PI_2);
            d[0] = v * 255;

            v = g + (27.0 - r * 27) / 255;
            v = MIN (1, v);
            d[1] = v * 255;

            v = g + (0.4 + b * 0.2 - sin(r));
            v = MIN (1, MAX (0, v));
            d[2] = v * 255;

            s += src_rgn.bpp;
            d += dest_rgn.bpp;
          }

          src += src_rgn.rowstride;
          dest += dest_rgn.rowstride;
        }
      }

      gimp_drawable_flush (drawable);
      gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
      gimp_drawable_update (drawable->drawable_id, x1, y1, width, height);*/

      break;
    }
    case BEAUTIFY_EFFECT_ICE_SPIRIT:
    {
      gdouble red_pts[] = {
        0.0, 0.007843, 0.121569, 0.141176,
        0.247059, 0.286275, 0.372549, 0.423529,
        0.498039, 0.552941, 0.623529, 0.674510,
        0.749020, 0.792157, 0.874510, 0.898039,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.0, 0.007843, 0.121569, 0.184314,
        0.247059, 0.360784, 0.372549, 0.517647,
        0.498039, 0.654902, 0.623529, 0.768627,
        0.749020, 0.866667, 0.874510, 0.945098,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.0, 0.007843, 0.121569, 0.211765,
        0.247059, 0.407843, 0.372549, 0.576471,
        0.498039, 0.717647, 0.623529, 0.827451,
        0.749020, 0.913725, 0.874510, 0.972549,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_JAPANESE_STYLE:
    {
      gdouble red_pts[] = {
        0.0, 0.098039, 0.121569, 0.188479,
        0.247059, 0.329761, 0.372549, 0.496682,
        0.498039, 0.657383, 0.623529, 0.787002,
        0.749020, 0.864444, 0.874510, 0.900704,
        1.000000, 0.917552,
      };
      gdouble green_pts[] = {
        0.0, 0.103431, 0.121569, 0.224676,
        0.247059, 0.394142, 0.372549, 0.541888,
        0.498039, 0.675963, 0.623529, 0.785613,
        0.749020, 0.893224, 0.874510, 0.943625,
        1.000000, 0.972720,
      };
      gdouble blue_pts[] = {
        0.0, 0.412025, 0.121569, 0.469119,
        0.247059, 0.615777, 0.372549, 0.751174,
        0.498039, 0.862955, 0.623529, 0.954468,
        0.749020, 0.995760, 0.874510, 1.000000,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_NEW_JAPANESE_STYLE:
    {
      gdouble red_pts[] = {
        0.0, 0.042969,
        0.350610, 0.320312,
        0.621951, 0.566406,
        0.847561, 0.632812,
        1.000000, 0.769531,
      };
      gdouble green_pts[] = {
        0.0, 0.031250,
        0.125000, 0.144531,
        0.500000, 0.523438,
        0.881098, 0.738281,
        1.000000, 0.882812,
      };
      gdouble blue_pts[] = {
        0.0, 0.0,
        0.121951, 0.039062,
        1.000000, 0.972656,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 10, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 10, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 6, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_MILK:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_milk, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 20, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      gdouble red_pts[] = {
        0.000000, 0.309804,
        0.121569, 0.392252,
        0.247059, 0.496494,
        0.372549, 0.597570,
        0.498039, 0.684511,
        0.623529, 0.768060,
        0.749020, 0.843330,
        0.874510, 0.917231,
        1.000000, 0.992213,
      };
      gdouble green_pts[] = {
        0.000000, 0.299346,
        0.121569, 0.395641,
        0.247059, 0.498204,
        0.372549, 0.597187,
        0.498039, 0.680991,
        0.623529, 0.762936,
        0.749020, 0.841731,
        0.874510, 0.910444,
        1.000000, 0.980896,
      };
      gdouble blue_pts[] = {
        0.000000, 0.296899,
        0.121569, 0.400465,
        0.247059, 0.501991,
        0.372549, 0.598690,
        0.498039, 0.685437,
        0.623529, 0.766052,
        0.749020, 0.842140,
        0.874510, 0.913442,
        1.000000, 0.981385,
      };
      layer = gimp_image_get_active_layer (image_ID);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_OLD_PHOTOS:
    {
      gdouble red_pts[] = {
        0.000000, 0.030719,
        0.121569, 0.094286,
        0.247059, 0.215434,
        0.372549, 0.446688,
        0.498039, 0.744585,
        0.623529, 0.854446,
        0.749020, 0.909348,
        0.874510, 0.948601,
        1.000000, 0.975656,
      };
      gdouble green_pts[] = {
        0.000000, 0.111275,
        0.121569, 0.172338,
        0.247059, 0.301636,
        0.372549, 0.462942,
        0.498039, 0.615155,
        0.623529, 0.736568,
        0.749020, 0.807584,
        0.874510, 0.874171,
        1.000000, 0.892938,
      };
      gdouble blue_pts[] = {
        0.000000, 0.333451,
        0.121569, 0.423250,
        0.247059, 0.517089,
        0.372549, 0.591683,
        0.498039, 0.631057,
        0.623529, 0.658333,
        0.749020, 0.678149,
        0.874510, 0.678550,
        1.000000, 0.677872,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_old_photos, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_WARM_YELLOW:
    {
      gdouble red_pts[] = {
        0.0, 0.000980, 0.121569, 0.065574,
        0.247059, 0.213677, 0.372549, 0.383298,
        0.498039, 0.556855, 0.623529, 0.726149,
        0.749020, 0.864046, 0.874510, 0.958157,
        1.000000, 0.996641,
      };
      gdouble green_pts[] = {
        0.0, 0.005882, 0.121569, 0.107837,
        0.247059, 0.276792, 0.372549, 0.452811,
        0.498039, 0.617782, 0.623529, 0.763782,
        0.749020, 0.886822, 0.874510, 0.965223,
        1.000000, 0.996993,
      };
      gdouble blue_pts[] = {
        0.0, 0.000495, 0.121569, 0.035825,
        0.247059, 0.149480, 0.372549, 0.305398,
        0.498039, 0.491352, 0.623529, 0.670305,
        0.749020, 0.838898, 0.874510, 0.951301,
        1.000000, 0.994118,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
    }
      break;
    case BEAUTIFY_EFFECT_BLUES:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.321569,
        0.247059, 0.541176,
        0.372549, 0.713725,
        0.498039, 0.831373,
        0.623529, 0.905882,
        0.749020, 0.952941,
        0.874510, 0.980392,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.266667,
        0.247059, 0.466667,
        0.372549, 0.627451,
        0.498039, 0.756863,
        0.623529, 0.847059,
        0.749020, 0.917647,
        0.874510, 0.964706,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.007843,
        0.121569, 0.286275,
        0.247059, 0.505882,
        0.372549, 0.682353,
        0.498039, 0.811765,
        0.623529, 0.901961,
        0.749020, 0.960784,
        0.874510, 0.988235,
        1.000000, 0.996078,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_COLD_BLUE:
    {
      gdouble red_pts[] = {
        0.000000, 0.000040,
        0.121569, 0.008043,
        0.247059, 0.066237,
        0.372549, 0.184771,
        0.498039, 0.359165,
        0.623529, 0.568527,
        0.749020, 0.772928,
        0.874510, 0.927927,
        1.000000, 0.993301,
      };
      gdouble green_pts[] = {
        0.000000, 0.001961,
        0.121569, 0.069848,
        0.247059, 0.214185,
        0.372549, 0.386385,
        0.498039, 0.560533,
        0.623529, 0.722231,
        0.749020, 0.865600,
        0.874510, 0.958973,
        1.000000, 0.996979,
      };
      gdouble blue_pts[] = {
        0.000000, 0.006332,
        0.121569, 0.225447,
        0.247059, 0.425338,
        0.372549, 0.585919,
        0.498039, 0.724790,
        0.623529, 0.833296,
        0.749020, 0.922240,
        0.874510, 0.975900,
        1.000000, 0.995237,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_COLD_GREEN:
    {
      gdouble red_pts[] = {
        0.000000, 0.000058,
        0.121569, 0.013332,
        0.247059, 0.092201,
        0.372549, 0.228389,
        0.498039, 0.407339,
        0.623529, 0.610095,
        0.749020, 0.797573,
        0.874510, 0.937331,
        1.000000, 0.993303,
      };
      gdouble green_pts[] = {
        0.000000, 0.008824,
        0.121569, 0.140109,
        0.247059, 0.324052,
        0.372549, 0.497982,
        0.498039, 0.655209,
        0.623529, 0.789734,
        0.749020, 0.900202,
        0.874510, 0.969304,
        1.000000, 0.997007,
      };
      gdouble blue_pts[] = {
        0.000000, 0.000495,
        0.121569, 0.035825,
        0.247059, 0.149480,
        0.372549, 0.305398,
        0.498039, 0.491352,
        0.623529, 0.670305,
        0.749020, 0.838898,
        0.874510, 0.951301,
        1.000000, 0.994118,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_PURPLE_FANTASY:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.184314,
        0.247059, 0.376471,
        0.372549, 0.533333,
        0.498039, 0.662745,
        0.623529, 0.788235,
        0.749020, 0.878431,
        0.874510, 0.941176,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.113725,
        0.247059, 0.243137,
        0.372549, 0.407843,
        0.498039, 0.623529,
        0.623529, 0.760784,
        0.749020, 0.847059,
        0.874510, 0.925490,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.007843,
        0.121569, 0.309804,
        0.247059, 0.505882,
        0.372549, 0.603922,
        0.498039, 0.709804,
        0.623529, 0.784314,
        0.749020, 0.854902,
        0.874510, 0.929412,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_COLD_PURPLE:
    {
      gdouble red_pts[] = {
        0.000000, 0.004412,
        0.121569, 0.137411,
        0.247059, 0.308078,
        0.372549, 0.470911,
        0.498039, 0.617224,
        0.623529, 0.745202,
        0.749020, 0.851077,
        0.874510, 0.936608,
        1.000000, 0.991056,
      };
      gdouble green_pts[] = {
        0.000000, 0.033824,
        0.121569, 0.044738,
        0.247059, 0.161690,
        0.372549, 0.319742,
        0.498039, 0.492767,
        0.623529, 0.662258,
        0.749020, 0.830133,
        0.874510, 0.940380,
        1.000000, 0.996802,
      };
      gdouble blue_pts[] = {
        0.000000, 0.140723,
        0.121569, 0.295178,
        0.247059, 0.504249,
        0.372549, 0.648502,
        0.498039, 0.757808,
        0.623529, 0.853601,
        0.749020, 0.928529,
        0.874510, 0.975401,
        1.000000, 0.992089,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_BRIGHT_RED:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_bright_red, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      gdouble red_pts[] = {
        0.000000, 0.001183,
        0.121569, 0.131140,
        0.247059, 0.353431,
        0.372549, 0.538498,
        0.498039, 0.690185,
        0.623529, 0.804008,
        0.749020, 0.900806,
        0.874510, 0.988271,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.000098,
        0.121569, 0.097098,
        0.247059, 0.286323,
        0.372549, 0.458654,
        0.498039, 0.611045,
        0.623529, 0.744221,
        0.749020, 0.858522,
        0.874510, 0.968540,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.000369,
        0.121569, 0.100320,
        0.247059, 0.285806,
        0.372549, 0.459693,
        0.498039, 0.612676,
        0.623529, 0.745027,
        0.749020, 0.860280,
        0.874510, 0.967918,
        1.000000, 1.000000,
      };
      layer = gimp_image_get_active_layer (image_ID);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
    case BEAUTIFY_EFFECT_CHRISTMAS_EVE:
    {
      gint32 layer = gimp_layer_new (image_ID, "color", width, height, GIMP_RGB_IMAGE, 100, GIMP_OVERLAY_MODE);
      gimp_image_add_layer (image_ID, layer, -1);
      GimpRGB color =
      {
        (gdouble) 156.0 / 255.0,
        (gdouble) 208.0 / 255.0,
        (gdouble) 240.0 / 255.0,
        1.0,
      };
      gimp_context_set_foreground (&color);
      gimp_edit_fill (layer, GIMP_FOREGROUND_FILL);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      GdkPixbuf *pixbuf = gdk_pixbuf_new_from_inline (-1, texture_christmas_eve, FALSE, NULL);
      gint32 texture_layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, texture_layer, -1);
      gimp_layer_scale (texture_layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, texture_layer, GIMP_CLIP_TO_BOTTOM_LAYER);
      break;
    }
    case BEAUTIFY_EFFECT_NIGHT_VIEW:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.181978,
        0.247059, 0.348183,
        0.372549, 0.498116,
        0.498039, 0.630567,
        0.623529, 0.746150,
        0.749020, 0.848447,
        0.874510, 0.939235,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.002451,
        0.121569, 0.182281,
        0.247059, 0.347678,
        0.372549, 0.495968,
        0.498039, 0.629733,
        0.623529, 0.747508,
        0.749020, 0.852278,
        0.874510, 0.937573,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.005240,
        0.121569, 0.182529,
        0.247059, 0.347919,
        0.372549, 0.496757,
        0.498039, 0.638620,
        0.623529, 0.753394,
        0.749020, 0.852676,
        0.874510, 0.948770,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_night_view, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_ASTRAL:
    {
      GdkPixbuf *pixbuf = gdk_pixbuf_new_from_inline (-1, texture_astral, FALSE, NULL);
      gint32 texture_layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SOFTLIGHT_MODE, 0, 0);
      gimp_image_add_layer (image_ID, texture_layer, -1);
      gimp_layer_scale (texture_layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, texture_layer, GIMP_CLIP_TO_BOTTOM_LAYER);
    }
      break;
    case BEAUTIFY_EFFECT_COLORFUL_GLOW:
    {
      gdouble red_pts[] = {
        0.000000, 0.151307,
        0.121569, 0.322524,
        0.247059, 0.448847,
        0.372549, 0.575643,
        0.498039, 0.686567,
        0.623529, 0.782251,
        0.749020, 0.867647,
        0.874510, 0.945974,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.141176,
        0.121569, 0.293870,
        0.247059, 0.430463,
        0.372549, 0.557343,
        0.498039, 0.673448,
        0.623529, 0.772324,
        0.749020, 0.849510,
        0.874510, 0.933274,
        1.000000, 0.991523,
      };
      gdouble blue_pts[] = {
        0.000000, 0.141529,
        0.121569, 0.292286,
        0.247059, 0.434369,
        0.372549, 0.557046,
        0.498039, 0.669468,
        0.623529, 0.772360,
        0.749020, 0.858682,
        0.874510, 0.932288,
        1.000000, 0.987544,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_colorful_glow, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_PICK_LIGHT:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      layer = gimp_layer_new (image_ID, "color", width, height, GIMP_RGB_IMAGE, 100, GIMP_SCREEN_MODE);
      gimp_image_add_layer (image_ID, layer, -1);
      GimpRGB color =
      {
        (gdouble) 62.0 / 255.0,
        (gdouble) 62.0 / 255.0,
        (gdouble) 62.0 / 255.0,
        1.0,
      };
      gimp_context_set_foreground (&color);
      gimp_edit_fill (layer, GIMP_FOREGROUND_FILL);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_pick_light_1, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture 1", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_pick_light_2, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture 2", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);
      break;
    }
    case BEAUTIFY_EFFECT_GLASS_DROPS:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_glass_drops, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_SKETCH:
    {
      gint32     layer;

      black_and_white (image_ID, effect_layer);

      layer = gimp_layer_copy (effect_layer);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_set_mode  (layer, GIMP_DODGE_MODE);
      gimp_invert (layer);

      GimpParam *return_vals;
      gint nreturn_vals;
      return_vals = gimp_run_procedure ("plug-in-gauss",
                                        &nreturn_vals,
                                        GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
                                        GIMP_PDB_IMAGE, image_ID,
                                        GIMP_PDB_DRAWABLE, layer,
                                        GIMP_PDB_FLOAT, 20.0,
                                        GIMP_PDB_FLOAT, 20.0,
                                        GIMP_PDB_INT32, 1,
                                        GIMP_PDB_END);
      gimp_destroy_params (return_vals, nreturn_vals);

      gimp_drawable_levels (layer, GIMP_HISTOGRAM_VALUE, 0.0, 1.0, 1, 1.0, 0.0, 0.98431372549, 1);

      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_LIFE_SKETCH:
    {
      gimp_drawable_desaturate (effect_layer, GIMP_DESATURATE_LUMINOSITY);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_life_sketch_1, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 60, GIMP_OVERLAY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_life_sketch_2, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      gdouble red_pts[] = {
        0.000000, 0.074510,
        0.121569, 0.105882,
        0.247059, 0.203391,
        0.372549, 0.325314,
        0.498039, 0.451172,
        0.623529, 0.582024,
        0.749020, 0.717003,
        0.874510, 0.855324,
        1.000000, 0.995987,
      };
      gdouble green_pts[] = {
        0.000000, 0.070588,
        0.121569, 0.102941,
        0.247059, 0.205857,
        0.372549, 0.330846,
        0.498039, 0.457491,
        0.623529, 0.591683,
        0.749020, 0.724243,
        0.874510, 0.859604,
        1.000000, 0.996069,
      };
      gdouble blue_pts[] = {
        0.000000, 0.062745,
        0.121569, 0.110294,
        0.247059, 0.223629,
        0.372549, 0.355159,
        0.498039, 0.490793,
        0.623529, 0.626121,
        0.749020, 0.753347,
        0.874510, 0.877884,
        1.000000, 0.996034,
      };
      layer = gimp_image_get_active_layer (image_ID);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      break;
    }
    case BEAUTIFY_EFFECT_CLASSIC_SKETCH:
    {
      gimp_drawable_desaturate (effect_layer, GIMP_DESATURATE_LUMINOSITY);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_classic_sketch_1, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_classic_sketch_2, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      gdouble red_pts[] = {
        0.000000, 0.035294,
        0.121569, 0.035294,
        0.247059, 0.100000,
        0.372549, 0.219095,
        0.498039, 0.373810,
        0.623529, 0.541166,
        0.749020, 0.697497,
        0.874510, 0.850874,
        1.000000, 0.996065,
      };
      gdouble green_pts[] = {
        0.000000, 0.015686,
        0.121569, 0.015686,
        0.247059, 0.085098,
        0.372549, 0.206671,
        0.498039, 0.357512,
        0.623529, 0.526512,
        0.749020, 0.687917,
        0.874510, 0.845067,
        1.000000, 0.996069,
      };
      gdouble blue_pts[] = {
        0.000000, 0.001961,
        0.121569, 0.001961,
        0.247059, 0.054390,
        0.372549, 0.180870,
        0.498039, 0.328319,
        0.623529, 0.497803,
        0.749020, 0.665890,
        0.874510, 0.835076,
        1.000000, 0.996066,
      };
      layer = gimp_image_get_active_layer (image_ID);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_classic_sketch_3, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_MULTIPLY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_COLOR_PENCIL:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_color_pencil, FALSE, NULL);

      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_OVERLAY_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_IMAGE);

      break;
    }
    case BEAUTIFY_EFFECT_TV_LINES:
    {
      gint32     layer;

      layer = gimp_layer_new (image_ID, "texture", width, height, GIMP_RGBA_IMAGE, 60, GIMP_MULTIPLY_MODE);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_drawable_fill (layer, GIMP_TRANSPARENT_FILL);
      gimp_context_set_pattern ("Stripes Fine");
      gimp_edit_fill (layer, GIMP_PATTERN_FILL);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_IMAGE);

      break;
    }
    case BEAUTIFY_EFFECT_RELIEF:
    {
      gint32     layer;

      layer = gimp_layer_copy (effect_layer);
      gimp_image_add_layer (image_ID, layer, -1);

      gimp_layer_set_offsets (effect_layer, 1, -1);
      gimp_layer_set_offsets (layer, -1, 1);

      gimp_layer_set_mode (layer, GIMP_GRAIN_EXTRACT_MODE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_IMAGE);

      //gimp_image_select_rectangle (image_ID, GIMP_CHANNEL_OP_REPLACE, 1, 1, width - 2, height - 2);
      gimp_rect_select (image_ID, 1, 1, width - 2, height - 2, GIMP_CHANNEL_OP_REPLACE, FALSE, 0);
      gimp_selection_invert (image_ID);

      layer = gimp_image_get_active_layer (image_ID);
      GimpRGB color = { 0.5, 0.5, 0.5, 1.0 };
      gimp_context_set_foreground (&color);
      gimp_edit_fill (layer, GIMP_FOREGROUND_FILL);

      gimp_selection_none (image_ID);

      break;
    }
    case BEAUTIFY_EFFECT_BEAM_GRADIENT:
    {
      gdouble red_pts[] = {
        0.000000, 0.003922,
        0.121569, 0.041976,
        0.247059, 0.156522,
        0.372549, 0.317488,
        0.498039, 0.499707,
        0.623529, 0.684590,
        0.749020, 0.845234,
        0.874510, 0.957891,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.005882,
        0.121569, 0.041838,
        0.247059, 0.156351,
        0.372549, 0.317066,
        0.498039, 0.499800,
        0.623529, 0.684389,
        0.749020, 0.843945,
        0.874510, 0.958236,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.005064,
        0.121569, 0.043674,
        0.247059, 0.157355,
        0.372549, 0.318250,
        0.498039, 0.499405,
        0.623529, 0.683918,
        0.749020, 0.844833,
        0.874510, 0.957540,
        1.000000, 1.000000,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_beam_gradient, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_SUNSET_GRADIENT:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_sunset_gradient, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_RAINBOW_GRADIENT:
    {
      gdouble red_pts[] = {
        0.000000, 0.024183,
        0.121569, 0.048308,
        0.247059, 0.154831,
        0.372549, 0.314038,
        0.498039, 0.497584,
        0.623529, 0.683345,
        0.749020, 0.841507,
        0.874510, 0.957243,
        1.000000, 0.997723,
      };
      gdouble green_pts[] = {
        0.000000, 0.010784,
        0.121569, 0.042665,
        0.247059, 0.155542,
        0.372549, 0.319728,
        0.498039, 0.499008,
        0.623529, 0.686299,
        0.749020, 0.843454,
        0.874510, 0.957699,
        1.000000, 0.999459,
      };
      gdouble blue_pts[] = {
        0.000000, 0.005853,
        0.121569, 0.043693,
        0.247059, 0.154759,
        0.372549, 0.318024,
        0.498039, 0.499930,
        0.623529, 0.681052,
        0.749020, 0.850097,
        0.874510, 0.959228,
        1.000000, 0.994901,
      };
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (effect_layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);

      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_rainbow_gradient, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_PINK_PURPLE_GRADIENG:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_pink_purple_gradient, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      break;
    }
    case BEAUTIFY_EFFECT_PINK_BLUE_GRADIENT:
    {
      gint32     layer;
      GdkPixbuf *pixbuf;

      pixbuf = gdk_pixbuf_new_from_inline (-1, texture_pink_blue_gradient, FALSE, NULL);
      layer = gimp_layer_new_from_pixbuf (image_ID, "texture", pixbuf, 100, GIMP_SCREEN_MODE, 0, 0);
      gimp_image_add_layer (image_ID, layer, -1);
      gimp_layer_scale (layer, width, height, FALSE);
      gimp_image_merge_down (image_ID, layer, GIMP_CLIP_TO_BOTTOM_LAYER);

      gdouble red_pts[] = {
        0.000000, 0.000392,
        0.121569, 0.038765,
        0.247059, 0.163053,
        0.372549, 0.298924,
        0.498039, 0.443050,
        0.623529, 0.612325,
        0.749020, 0.790453,
        0.874510, 0.939548,
        1.000000, 1.000000,
      };
      gdouble green_pts[] = {
        0.000000, 0.001569,
        0.121569, 0.044421,
        0.247059, 0.161120,
        0.372549, 0.300417,
        0.498039, 0.478406,
        0.623529, 0.664685,
        0.749020, 0.828593,
        0.874510, 0.954235,
        1.000000, 1.000000,
      };
      gdouble blue_pts[] = {
        0.000000, 0.260784,
        0.121569, 0.260784,
        0.247059, 0.260784,
        0.372549, 0.327074,
        0.498039, 0.447592,
        0.623529, 0.589356,
        0.749020, 0.776297,
        0.874510, 0.939428,
        1.000000, 1.000000,
      };
      layer = gimp_image_get_active_layer (image_ID);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_RED, 18, red_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_GREEN, 18, green_pts);
      gimp_drawable_curves_spline (layer, GIMP_HISTOGRAM_BLUE, 18, blue_pts);
      break;
    }
  }

  gimp_context_pop ();
}
