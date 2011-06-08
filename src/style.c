#include <stdarg.h>
#include <stdlib.h>

#include "map.h"
#include "style.h"
#include "util.h"

#define ST_CCEIL 256.0

typedef struct simplet_styledef_t {
  const char *key;
  void (*call)(cairo_t *ctx, const char *arg);
} simplet_styledef_t;

simplet_styledef_t styleTable[] = {
  { "fill",      simplet_style_fill      },
  { "stroke",    simplet_style_stroke    },
  { "weight",    simplet_style_weight    },
  { "line-join", simplet_style_line_join },
  { "line-cap",  simplet_style_line_cap  }
  /* radius and seamless are special styles */
};

const int STYLES_LENGTH = sizeof(styleTable) / sizeof(*styleTable);

static void
set_color(cairo_t *ctx, const char *arg){
  unsigned int r, g, b, a, count;
  count = sscanf(arg, "#%2x%2x%2x%2x", &r, &g, &b, &a);
  switch(count){
  case 3:
    cairo_set_source_rgb(ctx, r / ST_CCEIL, g / ST_CCEIL, b / ST_CCEIL);
    break;
  case 4:
    cairo_set_source_rgba(ctx, r / ST_CCEIL, g / ST_CCEIL, b / ST_CCEIL, a / ST_CCEIL);
    break;
  default:
    return;
  }
}

void
simplet_style_line_join(cairo_t *ctx, const char *arg){
  if(!strcmp("miter", arg))
    cairo_set_line_join(ctx, CAIRO_LINE_JOIN_MITER);
  if(!strcmp("round", arg))
    cairo_set_line_join(ctx, CAIRO_LINE_JOIN_ROUND);
  if(!strcmp("bevel", arg))
    cairo_set_line_join(ctx, CAIRO_LINE_JOIN_BEVEL);
}

void
simplet_style_line_cap(cairo_t *ctx, const char *arg){
  if(!strcmp("butt", arg))
    cairo_set_line_cap(ctx, CAIRO_LINE_CAP_BUTT);
  if(!strcmp("round", arg))
    cairo_set_line_cap(ctx, CAIRO_LINE_CAP_ROUND);
  if(!strcmp("square", arg))
    cairo_set_line_cap(ctx, CAIRO_LINE_CAP_SQUARE);
}

void
simplet_style_fill(cairo_t *ctx, const char *arg){
  set_color(ctx, arg);
  cairo_fill_preserve(ctx);
}

void
simplet_style_stroke(cairo_t *ctx, const char *arg){
  set_color(ctx, arg);
  cairo_stroke_preserve(ctx);
}

void
simplet_style_weight(cairo_t *ctx, const char *arg){
  double w = strtod(arg, NULL), y = 0;
  cairo_device_to_user_distance(ctx, &w, &y);
  cairo_set_line_width(ctx, w);
}


simplet_style_t*
simplet_style_new(const char *key, const char *arg){
  simplet_style_t* style;
  if(!(style = malloc(sizeof(*style))))
    return NULL;

  style->key = simplet_copy_string(key);
  style->arg = simplet_copy_string(arg);

  if(!(style->key && style->arg)){
    free(style);
    return NULL;
  }

  return style;
}

void
simplet_style_vfree(void *style){
  simplet_style_free(style);
}

void
simplet_style_free(simplet_style_t* style){
  free(style->key);
  free(style->arg);
  free(style);
}

static simplet_styledef_t*
lookup_styledef(char *key){
  for(int i = 0; i < STYLES_LENGTH; i++)
    if(!strcmp(key, styleTable[i].key))
      return &styleTable[i];
  return NULL;
}

void
simplet_apply_styles(cairo_t *ctx, simplet_list_t* styles, ...){
  va_list args;
  va_start(args, styles);
  char* key;
  while((key = va_arg(args, char*)) != NULL) {
    simplet_styledef_t* def = lookup_styledef(key);
    if(def == NULL)
      continue;

    simplet_style_t* style = simplet_lookup_style(styles, key);
    if(style == NULL)
      continue;

    def->call(ctx, style->arg);
  }
  va_end(args);
}

simplet_style_t*
simplet_lookup_style(simplet_list_t* styles, const char *key){
  simplet_listiter_t* iter;
  if(!(iter = simplet_get_list_iter(styles)))
    return NULL;

  simplet_style_t* style;
  while((style = simplet_list_next(iter)))
    if(!strcmp(key, style->key)) {
      simplet_list_iter_free(iter);
      return style;
    }


  return NULL;
}
