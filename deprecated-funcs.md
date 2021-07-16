## Unknown Fixes:

1. ‘gdk_pixbuf_new_from_inline’ is deprecated [-Wdeprecated-declarations]
2. ‘gimp_drawable_detach’ is deprecated [-Wdeprecated-declarations]

## Known-Fixes

1. ‘gimp_curves_spline’ is deprecated: Use 'gimp_drawable_curves_spline' instead [-Wdeprecated-declarations]
  1. **Resolved**
2. ‘gimp_desaturate_full’ is deprecated: Use 'gimp_drawable_desaturate' instead [-Wdeprecated-declarations]
  1. **Resolved**
3. ‘gimp_drawable_delete’ is deprecated: Use 'gimp_item_delete' instead [-Wdeprecated-declarations]
  1. **Resolved**
4. ‘gimp_drawable_get’ is deprecated: Use 'gimp_drawable_get_buffer' instead [-Wdeprecated-declarations]
  1. **Pending** - Complex refactor required
5. ‘gimp_hue_saturation’ is deprecated: Use 'gimp_drawable_hue_saturation' instead [-Wdeprecated-declarations]
  1. **Pending** - Refactor arguments from Integer ranges to Double ranges
6. ‘gimp_image_add_layer’ is deprecated: Use 'gimp_image_insert_layer' instead [-Wdeprecated-declarations]
  1. **Pending** - Likely simple fix, assess if new parent layer argument is relevant
7. ‘gimp_levels’ is deprecated: Use 'gimp_drawable_levels' instead [-Wdeprecated-declarations]
  1. **Resolved**
8. ‘gimp_rect_select’ is deprecated: Use 'gimp_image_select_rectangle' instead [-Wdeprecated-declarations]
