/*
        upic_popup.c
         Created           : 23-OCT-1989 by Christian Arnault
*/

#ifdef SCREEN 
//---------------------------------------------------------------------------
#include "UPI/upidef.h"
#include "SCR/scr.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

extern System Sys;
 
//---------------------------------------------------------------------------
void upic_open_pop_up (Menu* m, Param* p, int row, int col)   {
  Convert val;
  char* s = (char*) list_malloc (p->size + 1);
  Page* d = (Page*) list_malloc (sizeof(Page));
  d->lines = p->list_size;
  d->cols  = p->size;
  scrc_get_window_position (m->window, &d->row, &d->col);
  d->row   += row;
  d->col   += col;
  d->cur_line = 1;

  scrc_create_display (&d->id, d->lines, d->cols, NORMAL, ON, "List");
  for (int r=1; r<=d->lines; r++)  {
    if (p->type == REAL_FMT) val.d = *((double*) p->list + (r - 1));
    else val.i = *(p->list + (r - 1));
    upic_print_param (p, s, val);
    scrc_put_chars (d->id, s, NORMAL, r, 1, 0);
  }
  scrc_change_rendition (d->id, d->cur_line, 1, 1, d->cols, INVERSE);
  scrc_paste_display (d->id, Sys.pb, d->row, d->col);
  if (s) free (s);
  Sys.pop_up = d;
}

//---------------------------------------------------------------------------
void upic_close_pop_up () {
  scrc_delete_display (Sys.pop_up->id);
  free (Sys.pop_up);
  Sys.pop_up = (Page*) 0;
}

//---------------------------------------------------------------------------
void upic_set_pop_up_cursor ()    {
  return;
}

//---------------------------------------------------------------------------
void upic_act_on_pop_up (int *num, int key, int row)    {
  Page* d = Sys.pop_up;
  int delta, c = key, cur = d->cur_line;

  if (cur) scrc_change_rendition (d->id, cur, 1, 1, d->cols, NORMAL);
  scrc_begin_pasteboard_update (Sys.pb);
  switch (c)  {
    case PAGE_UP :
      delta = -12;
      if (delta < cur) delta = 1 - cur;
      break;
    case MOVE_UP :
      delta = -1;
      break;
    case PAGE_DOWN :
      delta = 12;
      if (delta >= d->lines - cur) delta = d->lines - cur - 1;
      break;
    case MOVE_DOWN :
      delta = 1;
      break;
    case 0 :
      delta = row - cur;
      break;
    default :
      if (cur) *num = cur - 1;
      upic_close_pop_up();
      break;
  }

  cur += delta;
  if (cur)  {
    if (cur > 0 && cur <= d->lines) d->cur_line = cur;
    else cur = d->cur_line;

    if (d->row + cur - 1 < 3 || d->row + cur - 1 > Sys.pb_rows - 2)  {
      d->row -= delta;
      scrc_move_display (d->id, Sys.pb, -delta, 0);
    }
    scrc_change_rendition (d->id, cur, 1, 1, d->cols, INVERSE);
  }
  else   {
     d->cur_line = cur;
  }
  /*  scrc_set_cursor (d->id, cur, 1);  */
  scrc_end_pasteboard_update (Sys.pb);
}
#else
static void dummy(void) { return; }
#endif
