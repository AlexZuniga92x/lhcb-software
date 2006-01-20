#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "SCR/scr.h"

#define BUFFER_GUARD 20

#define belongs_to(d,r,c) (r >= d->row0+d->row-1 && \
  r <= d->row1+d->row-1 && \
  c >= d->col0+d->col-1 && \
  c <= d->col1+d->col-1 )

#define visible(p,r,c) (r > 0 && r<=p->rows && c > 0 && c<=p->cols)

#define min(a,b) (a<b?a:b)

#define inverse()          scrc_putes("[7m", pb)
#define normal()           scrc_putes("[27m", pb)
#define clear_screen()     scrc_putes("[2J", pb)
#define underline()        scrc_putes("[4m", pb)
#define flash()            scrc_putes("[5m", pb)
#define plain()            scrc_putes("[0m", pb)
#define bold()             scrc_putes("[1m", pb)
#define beep()             scrc_puti (BELL, pb)
#define toascii()          scrc_putes("(B", pb)
#define tographic()        scrc_putes("(0", pb)
#define tosupp()           scrc_putes("(<", pb)
#define touser()           scrc_putes("(>", pb)
#define setfonts()         { scrc_putes("(B", pb);\
  scrc_putes(")0", pb);\
  scrc_putes("*<", pb);\
  scrc_putes("+>", pb); }
#define narrow_screen()    scrc_putes("[?3l", pb)
#define wide_screen()      scrc_putes("[?3h", pb)
#define cursor_on()        scrc_putes("[?25h", pb)
#define cursor_off()       scrc_putes("[?25l", pb)
#define setansi()          scrc_putes("=", pb)
#define resetansi()        scrc_putes(">", pb)
#define crlf()             {scrc_puti(CARRIAGERETURN, pb);\
  scrc_puti(LINEFEED, pb);}

static Pasteboard *Paste = 0;
static Display *Moving_display, *Moving_text;
static Display *Resizing_display, *Resizing_text;
int scr_ignore_input;

int scrc_create_pasteboard(Pasteboard** paste, char* device, int* rows, int* cols)  {
  if (Paste)  {
    *paste = Paste;
    *rows = Paste->rows;
    *cols = Paste->cols;
    return 0;
  }

  Pasteboard *pb = (Pasteboard *)list_malloc (sizeof(Pasteboard));
  Paste = pb;

  pb->bufout = (char *) list_malloc (BUFFER_SIZE);
  *(pb->bufout) = 0;
  pb->bufptr = 0;
  pb->bufsize = BUFFER_SIZE;

  if (device)  {
    pb->device = (char*) list_malloc (strlen(device) + 1);
    strcpy (pb->device, device);
  }
  else pb->device = (char *) 0;

  pb->rows = 24;
  pb->cols = 80;

  scrc_init_screen (pb, *rows, *cols);

  int size = pb->rows * pb->cols;
  *rows = pb->rows;
  *cols = pb->cols;

  list_init (&pb->paste);

  Update* u = (Update *) list_malloc (sizeof(Update));
  pb->upd = u;

  u->map  = (char *) list_malloc (size);
  u->attr = (char *) list_malloc (size);
  u->mod_rows = (char *) list_malloc (pb->rows);

  u = (Update *) list_malloc (sizeof(Update));
  pb->old = u;

  u->map  = (char *) list_malloc (size);
  u->attr = (char *) list_malloc (size);
  u->mod_rows = (char *) list_malloc (pb->rows);

  pb->updating = OFF;
  pb->old_attr = 0;

  pb->moving = 0;
  pb->resizing = 0;

  setansi();
  setfonts();
  scrc_clear_screen(pb);
  scrc_set_cursor_abs (pb, 1, 1);
  scrc_fflush (pb);

  scrc_create_display (&Moving_display, 2, 2, FLASH, OFF, "");
  scrc_put_chars (Moving_display, "\134/", NORMAL, 1, ON, 0);
  scrc_put_chars (Moving_display, "/\134", NORMAL, 2, ON, 0);

  scrc_create_display (&Moving_text, 5, 30, INVERSE, ON, "");
  scrc_put_chars (Moving_text, "    ^     :",
    NORMAL, 1, ON, 0);
  scrc_put_chars (Moving_text, "  <- ->   : Move the display",
    NORMAL, 2, ON, 0);
  scrc_put_chars (Moving_text, "    v     :",
    NORMAL, 3, ON, 0);
  scrc_put_chars (Moving_text, "Prev Next : Change order",
    NORMAL, 4, ON, 0);
  scrc_put_chars (Moving_text, "   PF3    : Validate",
    NORMAL, 5, ON, 0);

  scrc_create_display (&Resizing_display, 2, 2, FLASH, OFF, "");
  scrc_put_chars (Resizing_display, "/\134", NORMAL, 1, ON, 0);
  scrc_put_chars (Resizing_display, "\134/", NORMAL, 2, ON, 0);

  scrc_create_display (&Resizing_text, 5, 30, INVERSE, ON, "");
  scrc_put_chars (Resizing_text, "    ^     :",
    NORMAL, 1, ON, 0);
  scrc_put_chars (Resizing_text, "  <- ->   : Change the size",
    NORMAL, 2, ON, 0);
  scrc_put_chars (Resizing_text, "    v     :",
    NORMAL, 3, ON, 0);
  scrc_put_chars (Resizing_text, "Prev Next : Scroll",
    NORMAL, 4, ON, 0);
  scrc_put_chars (Resizing_text, "   PF4    : Validate",
    NORMAL, 5, ON, 0);

  *paste = pb;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_memory_of_pasteboard (Pasteboard *pb)  {
  int memory = sizeof(Pasteboard);
  Paste_entry *p = pb->paste.first;
  while (p)  {
    memory += sizeof (Paste_entry);
    p = p->next;
  }

  memory += pb->bufsize + 2 * sizeof(Update) + 4 * pb->rows * pb->cols +
    2 * pb->rows + 2 * pb->cols;

  if (pb->device) memory += strlen(pb->device) + 1;

  memory += scrc_memory_of_display (Moving_display) +
    scrc_memory_of_display (Moving_text) +
    scrc_memory_of_display (Resizing_display) +
    scrc_memory_of_display (Resizing_text);

  return (memory);
}

/*---------------------------------------------------------------------------*/
int scrc_get_pasteboard_infos (Pasteboard *pb, int* rows, int *cols)  {
  *rows = pb->rows;
  *cols = pb->cols;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_change_pasteboard (Pasteboard *pb, int *prows, int *pcols)   {
  Update *u;
  int size;

  scrc_clear_screen (pb);

  if (*pcols) pb->cols = *pcols;
  if (*prows) pb->rows = *prows;
  if (pb->cols > 80)   {
    wide_screen();
  }
  else  {
    narrow_screen();
  }

  /*  scrc_fflush(pb); */
  scrc_change_pbd_characteristics (pb);

  size = pb->rows * pb->cols;

  u = pb->upd;

  free (u->map);
  free (u->attr);
  free (u->mod_rows);

  u->map  = (char *) list_malloc (size);
  u->attr = (char *) list_malloc (size);
  u->mod_rows = (char *) list_malloc (pb->rows);

  u = pb->old;

  free (u->map);
  free (u->attr);
  free (u->mod_rows);

  u->map  = (char *) list_malloc (size);
  u->attr = (char *) list_malloc (size);
  u->mod_rows = (char *) list_malloc (pb->rows);

  scrc_repaint_screen (pb);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_delete_pasteboard (Pasteboard *pb) {
  Paste_entry *d;
  Update *u;

  if (pb->updating > 0) scrc_end_pasteboard_update (pb);

  while ((d = pb->paste.first))    {
    d->disp->paste = 0;
    list_remove_entry (d);
  }

  toascii();
  plain();
  cursor (pb, pb->rows,1);
  crlf ();
  resetansi ();
  scrc_fflush (pb);

  scrc_delete_physical_pasteboard (pb);

  u = pb->upd;

  free (u->map);
  free (u->attr);
  free (u->mod_rows);

  free (u);

  u = pb->old;

  free (u->map);
  free (u->attr);
  free (u->mod_rows);

  free (u);

  free (pb);
  Paste = 0;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_delete_display (Display *d)    {
  if (d->paste) scrc_unpaste_display (d, d->pb);
  free (d->map);
  free (d->attr);
  free (d);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_clear_screen (Pasteboard *pb)    {
  while (pb->updating > 0) scrc_end_pasteboard_update (pb);

  pb->old_attr = 0;
  clear_screen ();

  {
    Update *u;
    int size;

    u = pb->old;
    size = pb->rows * pb->cols;
    memset (u->map,  ' ', size);
#ifdef USE_FONT_SUPP
    memset (u->attr, FONT_SUPP, size);
#else
    memset (u->attr, NORMAL, size);
#endif
    memset (u->mod_rows, 0, pb->rows);
  }

  toascii();
  plain();
  cursor (pb, 1, 1);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_save_screen (Pasteboard *pb)   {
  scrc_resetANSI ();
  scrc_clear_screen (pb);
  scrc_set_scroll (pb, 1, pb->rows-3);
  scrc_fflush (pb);
  scr_ignore_input = 1;
  //_ss_rel(0);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_restore_screen (Pasteboard *pb)      {
  scrc_set_scroll (pb, 1, pb->rows);
  setfonts();
  scrc_setANSI();
  scrc_repaint_screen (pb);
  scr_ignore_input=0;
  scrc_ast_keyboard(0);
  scrc_save_screen_rearm();
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_repaint_screen (Pasteboard *pb)  {
  Paste_entry *d;
  Display *dd;

  scrc_clear_screen (pb);
  scrc_begin_pasteboard_update(pb);
  d = pb->paste.first;
  while (d)
  {
    dd = d->disp;
    scrc_draw_block (dd, dd->row0, dd->row1, dd->col0, dd->col1);
    d = d->next;
  }
  scrc_end_pasteboard_update(pb);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_create_display (Display** disp, int rows, int cols, int attr, flag border, char* title) {
  int size;
  Display *d = (Display *) list_malloc (sizeof(Display));
  d->pb = (Pasteboard *) 0;
  d->paste = (Paste_entry *) 0;
  d->rows  = rows;
  d->cols  = cols;
  d->row   = d->col  = 0;
  d->def_attr = attr;
  d->border = border;
  d->scroll = 0;
  d->resize = 0;
  d->drag = 0;
  d->wind = 0;
  rows += 2;
  cols += 2;
  size    = rows * cols;
  d->map  = (char *) list_malloc (size);
  d->attr = (char *) list_malloc (size);
  memset (d->map , SPACE, size);
  memset (d->attr, NORMAL, size);
  d->title  = 0;
  if (border)  {
    d->row0 = d->col0 = 0;
    d->row1 = d->rows + 1;
    d->col1 = d->cols + 1;
    d->height = rows;
    d->width  = cols;
    scrc_set_border (d, title, NORMAL);
  }
  else  {
    d->row0 = d->col0 = 1;
    d->row1 = d->rows;
    d->col1 = d->cols;
    d->height = rows - 2;
    d->width  = cols - 2;
  }
  *disp = d;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_memory_of_display (Display *disp)    {
  int memory = sizeof(Display) + 2 * (disp->rows + 2) * (disp->cols +2);
  return (memory);
}

/*---------------------------------------------------------------------------*/
int scrc_enable_scroll (Display *disp, int (*scroll)(int))   {
  disp->scroll = scroll;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_enable_resize (Display *disp, int (*resize)(Display*, int, int))   {
  disp->resize = resize;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_enable_drag (Display *disp, int (*drag)(Display*, int, int))   {
  disp->drag = drag;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_get_display_attr (Display *disp, int* rows, int* cols)   {
  if (disp)  {
    *rows = disp->rows;
    *cols = disp->cols;
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_read_from_display (Display *disp, char *string, int maxlen, int row)   {
  char *m;
  int cols;

  if (disp)
  {
    cols = disp->cols;
    m = disp->map + row * (cols + 2) + 1;
    if (maxlen > cols) maxlen = cols;
    strncpy (string, m, maxlen);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_set_border (Display *disp, char *title, char attr)   {
  int len, pos;
  int cols, rows;
  static char zero[] = "";

  if (disp->title) free (disp->title);

  len = (title) ? strlen(title) : 0;
  disp->title = (char *) list_malloc (len + 1);
  strcpy (disp->title, (title) ? title : zero);

  rows = disp->rows;
  cols = disp->cols;

  scrc_draw_box (disp, attr);
  if (cols > 1 && (len != 0))
  {
    if (len > cols) len = cols;
    pos = (cols - len + 2) >> 1;
    for (;len;len--,pos++)
      scrc_put_char_all (disp, pos, *title++, attr, 0, pos);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_paste_display (Display *disp, Pasteboard *pb, int row, int col)    {
  Paste_entry *d;
  level_up (pb);
  if ((d = disp->paste))  {
    if (row != disp->row || col != disp->col)
      scrc_move_display (disp, pb, row - disp->row, col - disp->col);
    if (d != pb->paste.last)
      list_move_entry (d, pb->paste.last, 0);
  }
  else
  {
    d = (Paste_entry *) list_add_entry (&pb->paste, sizeof(Paste_entry));
    scrc_uniconify_display (disp);
    disp->paste = d;
  }
  disp->pb       = pb;
  disp->row      = row;
  disp->col      = col;
  scrc_draw_block (disp, disp->row0, disp->row1, disp->col0, disp->col1);

  d->disp = disp;

  level_down (pb);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_unpaste_display (Display *disp, Pasteboard *pb)    {
  if (disp && disp->paste)    {
    level_up (pb);
    scrc_undraw_block (disp, disp->row0, disp->row1, disp->col0, disp->col1);
    list_remove_entry (disp->paste);
    disp->paste = 0;
    level_down (pb);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_bring_display_to_back (Display *disp, Pasteboard *pb)    {
  Paste_entry *d = disp->paste;
  if (!d) return 0;

  if (d != pb->paste.first)  {
    level_up (pb);
    list_move_entry (d, 0, pb->paste.first);
    d = pb->paste.first;
    while (d)      {
      Display *dd = d->disp;
      scrc_draw_block (dd, dd->row0, dd->row1, dd->col0, dd->col1);
      d = d->next;
    }
    level_down (pb);
  }
  return 1;
}     

/*---------------------------------------------------------------------------*/
int scrc_move_display (Display *disp, Pasteboard *pb, int drow, int dcol) {
  level_up (pb);
  scrc_undraw_block (disp, disp->row0, disp->row1, disp->col0, disp->col1);
  disp->row += drow;
  disp->col += dcol;
  scrc_draw_block (disp, disp->row0, disp->row1, disp->col0, disp->col1);
  level_down (pb);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_change_display (Display *disp, int rows, int cols)   {
  int w, h;
  int r, c;
  char *map, *m, *mm;
  char *attr, *a, *aa;
  int size = (rows + 2)*(cols + 2);
  int r0, r1, c0, c1;
  Pasteboard *pb = disp->pb;
  int offset;

  h = disp->rows;
  w = disp->cols;

  if (h == rows && w == cols) return 1;

  level_up (pb);
  m = disp->map;
  a = disp->attr;

  if (disp->border)
  {
    r0 = c0 = 0;
    r1 = rows + 1;
    c1 = cols + 1;
  }
  else
  {
    r0 = c0 = 1;
    r1 = rows;
    c1 = cols;
  }

  disp->map  = map  = (char *) list_malloc (size);
  disp->attr = attr = (char *) list_malloc (size);
  memset (map,  ' ', size);
  memset (attr, disp->def_attr, size);
  if (rows < disp->rows)
    scrc_undraw_block (disp, r1+1, disp->row1, disp->col0, disp->col1);
  if (cols < disp->cols)
    scrc_undraw_block (disp, r0, r1, c1+1, disp->col1);

  disp->rows = rows;
  disp->cols = cols;
  disp->row1 = r1;
  disp->col1 = c1;
  if (disp->border)
  {
    disp->height = rows + 2;
    disp->width  = cols + 2;
  }
  else
  {
    disp->height = rows;
    disp->width  = cols;
  }

  mm = m + w + 3;
  aa = a + w + 3;


  for (r = 1; r <= min(h,rows); r++)
  {
    offset = r * (cols + 2) + 1;

    for (c = 1; c <= min(w,cols); c++, offset++)
    {
      scrc_put_char_all (disp, offset, *mm++, *aa++, r, c);
    }
    for (; c<=cols; c++, offset++)
    {
      scrc_put_char_all (disp, offset, ' ', NORMAL, r, c);
    }

    if (w > cols)
    {
      mm += w - cols;
      aa += w - cols;
    }
    mm += 2;
    aa += 2;
  }
  for (; r<=rows; r++)
  {
    offset = r*(cols + 2) + 1;
    for (c = 1; c <= cols; c++, offset++)
      scrc_put_char_all (disp, offset, ' ', NORMAL, r, c);
  }

  if (disp->border) scrc_set_border (disp, disp->title, NORMAL);

  if (rows > h) scrc_draw_block (disp, h+1, r1, c0, c1);
  if (cols > w) scrc_draw_block (disp, r0, r1, w+1, c1);

  free (m);
  free (a);
  level_down (pb);
  return 1;
}

/*---------------------------------------------------------------------------*/
char *scrc_get_title (Display *disp)    {
  if (disp->border) return (disp->title);
  return (0);
}

/*---------------------------------------------------------------------------*/
int scrc_draw_block (Display *disp, int r1, int r2, int c1, int c2) {
  int r, c;
  int row, col;
  int offset, pb_offset;
  Pasteboard *pb;
  int cols;

  if (!disp->paste) return 1;

  pb   = disp->pb;
  cols = disp->cols + 2;

  row = disp->row;
  col = disp->col;

  r = r1;
  c = c1;

  r1 += row - 1;
  if (r1 < 1)
  {
    r -= r1 - 1;
    r1 = 1;
  }
  if (r1 > pb->rows) return (1);

  r2 += row - 1;
  if (r2 < 1) return (1);
  if (r2 > pb->rows) r2 = pb->rows;

  c1 += col - 1;
  if (c1 < 1)
  {
    c -= c1 - 1;
    c1 = 1;
  }
  if (c1 > pb->cols) return (1);

  c2 += col - 1;
  if (c2 < 1) return (1);
  if (c2 > pb->cols) c2 = pb->cols;

  offset = r*cols + c;

  for (r=r1; r<=r2; r++)
  {
    char *map, *attr;

    map    = disp->map + offset;
    attr   = disp->attr + offset;

    pb_offset = (r - 1)*pb->cols + c1 - 1;
    for (c=c1; c<=c2; c++, pb_offset++)
    {
      scrc_draw_char (disp, pb, pb_offset, *map, *attr, disp, r, c);
      map++;
      attr++;
    }
    offset += cols;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
int scrc_undraw_block (Display *disp, int r1, int r2, int c1, int c2)   {
  int r, c;
  int row, col;
  int offset, pb_offset;
  Paste_entry *d;
  Display *dd;
  Pasteboard *pb;
  int cols;

  if (!disp->paste) return 1;

  pb   = disp->pb;
  cols = disp->cols + 2;

  row = disp->row;
  col = disp->col;

  r1 += row - 1;
  if (r1 < 1) r1 = 1;
  if (r1 > pb->rows) return (1);

  r2 += row - 1;
  if (r2 < 1) return (1);
  if (r2 > pb->rows) r2 = pb->rows;

  c1 += col - 1;
  if (c1 < 1) c1 = 1;
  if (c1 > pb->cols) return (1);

  c2 += col - 1;
  if (c2 < 1) return (1);
  if (c2 > pb->cols) c2 = pb->cols;

  for (r=r1; r<=r2; r++)
  {
    pb_offset = (r - 1)*pb->cols + c1 - 1;
    for (c=c1; c<=c2; c++, pb_offset++)
    {
      int done;

      done = 0;
      d = disp->paste->prev;
      while (d)
      {
        dd = d->disp;
        if (belongs_to(dd, r, c))
        {
          offset = (r - dd->row + 1)*(dd->cols + 2) + c - dd->col + 1;
          scrc_draw_char (disp, pb, pb_offset, *(dd->map + offset),
            *(dd->attr + offset), dd, r, c);
          break;
        }
        d = d->prev;
      }
#ifdef USE_FONT_SUPP
      if (!d) scrc_draw_char (disp, pb, pb_offset, ' ', FONT_SUPP, 0, r, c);
#else
      if (!d) scrc_draw_char (disp, pb, pb_offset, ' ', NORMAL, 0, r, c);
#endif
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
Display* scrc_display_at (Pasteboard* pb, int row, int col)    {
  Paste_entry *d = pb->paste.last;
  while (d)  {
    Display* dd = d->disp;
    if ( belongs_to(dd, row, col) )  return dd;
    d = d->prev;
  }
  return (0);
}

/*---------------------------------------------------------------------------*/
int scrc_draw_box (Display *disp, char attr)    {
  int r, c, offset;
  int r2, c2;

  attr |= GRAPHIC;

  r2 = disp->rows + 1;
  c2 = disp->cols + 1;

  scrc_put_char_all (disp, 0, TOP_LEFT_CORNER, attr, 0, 0);
  for (c = 1, offset = 1; c < c2; c++, offset++)
  {
    scrc_put_char_all (disp, offset, HORIZONTAL_BAR, attr, 0, c);
  }
  scrc_put_char_all (disp, offset, TOP_RIGHT_CORNER, attr, 0, c);
  offset++;

  for (r = 1; r < r2; r++)
  {
    scrc_put_char_all (disp, offset, VERTICAL_BAR, attr, r, 0);
    offset += c2;
    scrc_put_char_all (disp, offset, VERTICAL_BAR, attr, r, c2);
    offset++;
  }

  scrc_put_char_all (disp, offset, BOTTOM_LEFT_CORNER, attr, r, 0);
  offset++;
  for (c = 1; c < c2; c++, offset++)
  {
    scrc_put_char_all (disp, offset, HORIZONTAL_BAR, attr, r, c);
  }
  scrc_put_char_all (disp, offset, BOTTOM_RIGHT_CORNER, attr, r, c);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_display_occluded (Display *disp)   {
  for (int row = disp->row0 + disp->row; row <= disp->row1 + disp->row; row++)
    for (int col = disp->col0 + disp->col; col <= disp->col1 + disp->col; col++)
      if (scrc_occluded (disp, row, col)) return 1;
  return 0;
}

/*---------------------------------------------------------------------------*/
int scrc_occluded (Display *disp, int row, int col)   {
  Paste_entry *d;
  Display *dd;

  d = disp->paste;
  if (!d) return 0;

  d = d->next;
  while (d)
  {
    dd = d->disp;
    if (belongs_to(dd, row, col)) return 1;
    d = d->next;
  };
  return 0;
}

/*---------------------------------------------------------------------------*/
int scrc_put_chars (Display *disp, char *str, byte attr, int row, int col, int erase) {
  int len;
  int w, h;

  h = disp->rows;
  w = disp->cols;

  if (row < 0 || col < 0 || row > h || col > w) return 0;
  len = strlen(str);
  if (len > w - col + 1) len = w - col + 1;
  for (;len;len--) scrc_put_char (disp, *str++, attr, row, col++);
  if (erase)
    for (; col <= w; col++)
      scrc_put_char (disp, ' ', attr, row, col);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_erase_line (Display *disp, int row)  {
  int i;

  if (row > disp->rows) return 0;
  for (i = 1;i <= disp->cols; i++) scrc_put_char (disp, ' ', NORMAL, row, i);

  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_put_char_all (Display *disp, int offset, char c, unsigned char attr, int row, int col)   {
  int pb_offset;

  *(disp->map  + offset) = c;
  *(disp->attr + offset) = attr;

  if (disp->paste)
  {
    Pasteboard *pb = disp->pb;

    row += disp->row - 1;
    col += disp->col - 1;
    if (visible (pb, row, col))
    {
      pb_offset = (row - 1)*pb->cols + col - 1;
      scrc_draw_char (disp, pb, pb_offset, c, attr, disp, row, col);
    }
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_put_char (Display *disp, char c, unsigned char attr, int row, int col)   {
  int w, h, offset, pb_offset;

  h = disp->rows;
  w = disp->cols;

  if (row < 0 || row > h+1 || col < 0 || col > w+1) return 0;

  w += 2;
  offset = row*w + col;
  *(disp->map  + offset) = c < ' ' ? ' ' : c;
  *(disp->attr + offset) = attr;

  if (disp->paste)
  {
    Pasteboard *pb = disp->pb;

    row += disp->row - 1;
    col += disp->col - 1;
    if (visible (pb, row, col))
    {
      pb_offset = (row - 1)*pb->cols + col - 1;
      scrc_draw_char (disp, pb, pb_offset, c, attr, disp, row, col);
    }
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_get_char (Display *disp, int row, int col, char* c, unsigned int* attr)  {
  int w, h, offset;

  h = disp->rows;
  w = disp->cols;

  if (row > h+1 || col > w+1) return 0;

  w += 2;
  offset = row*w + col;
  *c = *(disp->map  + offset);
  *attr = *(disp->attr + offset);
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_draw_char (Display *disp, Pasteboard *pb, int offset, 
                    char c, unsigned char attr, Display *ddef, int row, int col)
{
  char *m, *a, *mm, *aa;
  Update *u;
  int def;

  if (!pb) return 0;

  /*  Check to use the default display attribute                 */

  def = NORMAL;
  if (ddef) def = ddef->def_attr;


  /*  Check occlusion of this position within the display        */

  if (disp && scrc_occluded (disp, row, col)) return 0;


  /*  Take care of multiple inversions                           */

  attr = (~INVERSE & (def | attr)) | (INVERSE & (def ^ attr)) ;


  /*  Further checks on the char itself                          */

  if (!c) c = ' ';
  if (c == ' ')
  {
#ifdef USE_FONT_SUPP
    attr &= ATTRIBUTES | FONT_SUPP;
#else
    attr &= ATTRIBUTES;
#endif
  }

  u = pb->old;
  mm = u->map + offset;
  aa = u->attr + offset;

  if (pb->updating > 0)  {
    u = pb->upd;
    m = u->map + offset;
    a = u->attr + offset;
    if (c != *mm || attr != *aa)    {
      u->mod_rows[row-1] = MODIFIED;
    }
    *m = c;
    *a = attr | MODIFIED;
  }
  else  {
    if (*mm == c && *aa == attr) return 1;
    *mm = c;
    *aa = attr;
    scrc_draw_char_on_pb (pb, c, attr, row, col);
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_draw_char_on_pb (Pasteboard *pb, char c, int attr, int row, int col)   {
  byte old;
  int old_set, set;

  old = pb->old_attr;

  old_set = old & FONTS;
  set     = attr & FONTS;
  if (old_set != set)
  {
    switch (set)
    {
    case ASCII : toascii();
      break;
    case GRAPHIC : tographic();
      break;
    case FONT_SUPP : tosupp();
      break;
    case FONT_USER : touser();
      break;
    }
    old &= ~FONTS;
    old |= set;
  }

  old_set = old & ATTRIBUTES;
  set     = attr & ATTRIBUTES;
  if (old_set)
  {
    if (set != old_set)
    {
      plain();
      old &= ~ATTRIBUTES;
    }
    else set = 0;
  }
  if (set)
  {
    if (set & BOLD)      bold();
    if (set & UNDERLINE) underline();
    if (set & FLASH)     flash();
    if (set & INVERSE)   inverse();
    old |= set;
  }
  if (row != pb->curs.row || col != pb->curs.col)
    scrc_set_cursor_abs (pb, row, col);
  scrc_putc (c, pb);
  pb->curs.col++;
  pb->old_attr = old;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_set_cursor_abs (Pasteboard *pb, int row, int col)  {
  cursor (pb, row, col);
  pb->curs.row = row;
  pb->curs.col = col;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_set_cursor (Display *disp, int row, int col)   {
  if (disp->paste)
    scrc_set_cursor_abs (disp->pb, disp->row + row - 1, disp->col + col - 1);
  return 1;
}


/*---------------------------------------------------------------------------*/
int scrc_check_key_buffer (char *buffer)
/*---------------------------------------------------------------------------*/
/* This function checks the buffer for a valid escape sequence or a normal   */
/* key stroke.                                                               */
/*  If a valid key sequence is found, the corresponding code is returned.    */
/*  INVALID may be returned.                                                 */
/*  (-1)    is returned if the escape sequence is incomplete.                */
/*---------------------------------------------------------------------------*/
{
  int b;
  char c;

  b = *buffer & 0xff;
  switch (b)
  {
  case 0x9b:
    buffer++;
    switch (*buffer)
    {
    case 'D': return MOVE_LEFT;
    case 'B': return MOVE_DOWN;
    case 'A': return MOVE_UP;
    case 'C': return MOVE_RIGHT;
    case '1':
      buffer++;
      switch (c = *buffer)
      {
      case '~' : return KPD_FIND;
      case '7' :
      case '8' :
      case '9' :
        buffer++;
        switch (*buffer)
        {
        case '~' :
          switch (c)
          {
          case '7' : return F6;
          case '8' : return F7;
          case '9' : return F8;
          }
          break;
        case 0 : return (-1);
        }
        break;
      case 0 : return (-1);
      }
      break;
    case '2':
      buffer++;
      switch (c = *buffer)
      {
      case '~' : return KPD_INSERT;
      case '0' :
      case '1' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '8' :
      case '9' :
        buffer++;
        switch (*buffer)
        {
        case '~' :
          switch (c)
          {
          case '0' : return F9;
          case '1' : return F10;
          case '3' : return F11;
          case '4' : return F12;
          case '5' : return F13;
          case '6' : return F14;
          case '8' : return F15;
          case '9' : return F16;
          }
          break;
        case 0 : return (-1);
        }
        break;
      case 0 : return (-1);
      }
      break;
    case '3':
      buffer++;
      switch (c = *buffer)
      {
      case '~' : return KPD_REMOVE;
      case '1' :
      case '2' :
      case '3' :
      case '4' :
        buffer++;
        switch (*buffer)
        {
        case '~' :
          switch (c)
          {
          case '1' : return F17;
          case '2' : return F18;
          case '3' : return F19;
          case '4' : return F20;
          }
          break;
        case 0 : return (-1);
        }
        break;
      case 0 : return (-1);
      }
      break;
    case '4':
      buffer++;
      switch (c = *buffer)
      {
      case '~' : return KPD_SELECT;
      case 0 : return (-1);
      }
      break;
    case '5':
      buffer++;
      switch (c = *buffer)
      {
      case '~' : return KPD_PREV;
      case 0 : return (-1);
      }
      break;
    case '6':
      buffer++;
      switch (c = *buffer)
      {
      case '~' : return KPD_NEXT;
      case 0 : return (-1);
      }
      break;
    case 0:
      return (-1);
      break;
    }
    break;
  case 0x8f :
    buffer++;
    switch (*buffer)
    {
    case 'l': return PAGE_DOWN;
    case 'm': return PAGE_UP;
    case 'n': return KPD_PERIOD;
    case 'p': return KPD_0;
    case 'q': return KPD_1;
    case 'r': return KPD_2;
    case 's': return KPD_3;
    case 't': return KPD_4;
    case 'u': return KPD_5;
    case 'v': return KPD_6;
    case 'w': return KPD_7;
    case 'x': return KPD_8;
    case 'y': return KPD_9;
    case 'M': return KPD_ENTER;
    case 'P': return KPD_PF1;
    case 'Q': return KPD_PF2;
    case 'R': return KPD_PF3;
    case 'S': return KPD_PF4;
    case 0: return (-1);
    }
    break;
  case 0x1b :
    buffer++;
    switch (*buffer)
    {
    case '[':
      buffer++;
      switch (*buffer)
      {
      case 'D': return MOVE_LEFT;
      case 'B': return MOVE_DOWN;
      case 'A': return MOVE_UP;
      case 'C': return MOVE_RIGHT;
      case '1':
        buffer++;
        switch (c = *buffer)
        {
        case '~' : return KPD_FIND;
        case '7' :
        case '8' :
        case '9' :
          buffer++;
          switch (*buffer)
          {
          case '~' :
            switch (c)
            {
            case '7' : return F6;
            case '8' : return F7;
            case '9' : return F8;
            }
          case 0 : return (-1);
          }
          break;
        case 0 : return (-1);
        }
        break;
      case '2':
        buffer++;
        switch (c = *buffer)
        {
        case '~' : return KPD_INSERT;
        case '0' :
        case '1' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '8' :
        case '9' :
          buffer++;
          switch (*buffer)
          {
          case '~' :
            switch (c)
            {
            case '0' : return F9;
            case '1' : return F10;
            case '3' : return F11;
            case '4' : return F12;
            case '5' : return F13;
            case '6' : return F14;
            case '8' : return F15;
            case '9' : return F16;
            }
            break;
          case 0 : return (-1);
          }
          break;
        case 0 : return (-1);
        }
        break;
      case '3':
        buffer++;
        switch (c = *buffer)
        {
        case '~' : return KPD_REMOVE;
        case '1' :
        case '2' :
        case '3' :
        case '4' :
          buffer++;
          switch (*buffer)
          {
          case '~' :
            switch (c)
            {
            case '1' : return F17;
            case '2' : return F18;
            case '3' : return F19;
            case '4' : return F20;
            }
            break;
          case 0 : return (-1);
          }
          break;
        case 0 : return (-1);
        }
        break;
      case '4':
        buffer++;
        switch (c = *buffer)
        {
        case '~' : return KPD_SELECT;
        case 0 : return (-1);
        }
        break;
      case '5':
        buffer++;
        switch (c = *buffer)
        {
        case '~' : return KPD_PREV;
        case 0 : return (-1);
        }
        break;
      case '6':
        buffer++;
        switch (c = *buffer)
        {
        case '~' : return KPD_NEXT;
        case 0 : return (-1);
        }
        break;
      case 0 :
        return (-1);
        break;
      }
      break;
    case 'O':
      buffer++;
      switch (*buffer)
      {
      case 'l': return PAGE_DOWN;
      case 'm': return PAGE_UP;
      case 'n': return KPD_PERIOD;
      case 'p': return KPD_0;
      case 'q': return KPD_1;
      case 'r': return KPD_2;
      case 's': return KPD_3;
      case 't': return KPD_4;
      case 'u': return KPD_5;
      case 'v': return KPD_6;
      case 'w': return KPD_7;
      case 'x': return KPD_8;
      case 'y': return KPD_9;
      case 'M': return KPD_ENTER;
      case 'P': return KPD_PF1;
      case 'Q': return KPD_PF2;
      case 'R': return KPD_PF3;
      case 'S': return KPD_PF4;
      case 0 : return (-1);
      }
      break;
    case 0:
      return (-1);
      break;
    }
    break;
  case 0x7f :
    return DELETE;

  default:
    if (b < 0x20) return (INVALID + b);
    else if (b <= '~') return b;
  }
  return INVALID;
}

/*---------------------------------------------------------------------------*/
int scrc_ring_bell (Display *d)  {
  Pasteboard* pb = d->pb;
  if ( pb ) beep ();
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_load_font (Pasteboard *pb, char *name)   {
  char buf[80];
  FILE* f = fopen (name, "r");
  if (!f) return 0;
  scrc_fflush(pb);
  while (fgets (buf, 80, f)) scrc_putes (buf, pb);
  fclose(f);
  setfonts();
  return scrc_fflush(pb);
}

/*---------------------------------------------------------------------------*/
int scrc_resizing_display (Display *d)    {
  Pasteboard *pb = d->pb;
  pb->resizing = d;

  scrc_paste_display (Resizing_text, pb, 2, 2);
  scrc_paste_display (Resizing_display, pb, d->row, d->col);
  cursor_off();
  return scrc_end_pasteboard_update (pb);
}


/*---------------------------------------------------------------------------*/
int scrc_action_resizing_display (Pasteboard *pb, int key)    {
  Display *d;
  int done;
  int drow, dcol;

  d = pb->resizing;

  done = 0;
  scrc_begin_pasteboard_update (pb);
  dcol = 0;
  drow = 0;
  switch (key)
  {
  case MOVE_UP :
    if (Resizing_display->row > 1) drow = -1;
    break;
  case MOVE_DOWN :
    if (Resizing_display->row < pb->rows) drow = 1;
    break;
  case MOVE_LEFT :
    if (Resizing_display->col > 2) dcol = -1;
    break;
  case MOVE_RIGHT :
    if (Resizing_display->col < pb->cols) dcol = 1;
    break;
  case KPD_PREV :
  case KPD_NEXT :
    if (d->scroll)
    {
      (*d->scroll)(key);
      scrc_paste_display (Resizing_display, pb, d->row, d->col);
      dcol = 0;
      drow = 0;
    }
    break;
  default :
    done = 1;
    break;
  }
  if (!done) scrc_move_display (Resizing_display, pb, drow, dcol);
  else
  {
    pb->resizing = 0;
    scrc_unpaste_display (Resizing_display, pb);
    scrc_unpaste_display (Resizing_text, pb);
    cursor_on();
    if (key == KPD_PF4 || key == KPD_PF3 )
    {
      drow = Resizing_display->row - d->row;
      dcol = Resizing_display->col - d->col;
      if (drow || dcol)
      {
        if (d->resize) (*d->resize)(d, d->rows - drow, d->cols - dcol);
        if (d->drag)   (*d->drag)(d, drow, dcol);
      }
    }
  }
  scrc_end_pasteboard_update(pb);
  return (done);
}


/*---------------------------------------------------------------------------*/
int scrc_moving_display (Display *d)    {
  Pasteboard *pb;

  pb = d->pb;
  pb->moving = d;

  scrc_paste_display (Moving_text, pb, pb->rows-5, 2);
  scrc_paste_display (Moving_display, pb, d->row, d->col);
  cursor_off();
  return scrc_end_pasteboard_update (pb);
}


/*---------------------------------------------------------------------------*/
int scrc_action_moving_display (Pasteboard *pb, int key)    {
  Display *d = pb->moving;
  int done = 0;
  int dcol = 0;
  int drow = 0;

  scrc_begin_pasteboard_update (pb);
  switch (key)
  {
  case MOVE_UP :
    if (Moving_display->row > 1) drow = -1;
    break;
  case MOVE_DOWN :
    if (Moving_display->row < pb->rows) drow = 1;
    break;
  case MOVE_LEFT :
    if (Moving_display->col > 2) dcol = -1;
    break;
  case MOVE_RIGHT :
    if (Moving_display->col < pb->cols) dcol = 1;
    break;
  case KPD_NEXT :
  case KPD_PREV :
    if (d->scroll)
    {
      (*d->scroll)(key);
      scrc_paste_display (Moving_display, pb, d->row, d->col);
      dcol = 0;
      drow = 0;
    }
    break;
  default :
    done = 1;
    break;
  }
  if (!done)    {
    scrc_move_display (Moving_display, pb, drow, dcol);
  }
  else
  {
    pb->moving = 0;
    scrc_unpaste_display (Moving_display, pb);
    scrc_unpaste_display (Moving_text, pb);
    cursor_on();
    if (key == KPD_PF3 || key == KPD_PF4)
    {
      drow = Moving_display->row - d->row;
      dcol = Moving_display->col - d->col;
      if (drow || dcol)
      {
        /*
        if ((drag = d->drag))
        (*drag) (d, Moving_display->row, Moving_display->col);
        */
        if (d->drag)        {
          (*d->drag) (d, drow, dcol);
        }
        else scrc_move_display (d, pb, drow, dcol);
      }
    }
  }
  scrc_end_pasteboard_update(pb);
  return (done);
}


/*----------------------------------------------------------------------------*/
int scrc_fflush(Pasteboard *pb)    {
  char *buf;

  if (pb->bufptr)
  {
    buf = pb->bufout + pb->bufptr;
    *buf = 0;
    scrc_fputs(pb);
    pb->bufptr = 0;
    *(pb->bufout) = 0;
  }
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_save_cursor(Pasteboard *pb, int *(context[]))    {
  /*
  *context = (int *) list_malloc (3*4);

  (*context)[0] = pb->curs.row;
  (*context)[1] = pb->curs.col;
  (*context)[2] = pb->old_attr;

  if (pb->updating <= 0) scrc_putes ("7", pb);
  */
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_restore_cursor(Pasteboard *pb, int context[])    {
  /*
  pb->curs.row = context[0];
  pb->curs.col = context[1];
  pb->old_attr = context[2];

  if (pb->updating > 0) cursor (pb, pb->curs.row, pb->curs.col);
  else
  {
  scrc_putes ("8", pb);
  scrc_fflush (pb);
  }
  free (context);
  */
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_putc (char c, Pasteboard *pb)    {
  char *buf;

  buf = pb->bufout + pb->bufptr;
  *buf = c;
  buf++;
  *buf = 0;
  pb->bufptr++;
  if (pb->bufptr > pb->bufsize - BUFFER_GUARD) scrc_fflush (pb);
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_putes (char* s, Pasteboard *pb)    {
  char *buf;

  buf = pb->bufout + pb->bufptr;
  *buf = ESCAPE;
  pb->bufptr++;
  return scrc_puts (s, pb);
}

/*----------------------------------------------------------------------------*/
int scrc_puts (char* s, Pasteboard *pb)   {
  char *buf;

  buf = pb->bufout + pb->bufptr;
  strcpy (buf, s);
  pb->bufptr += strlen(s);
  if (pb->bufptr > pb->bufsize - BUFFER_GUARD) scrc_fflush (pb);
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_puti (int i, Pasteboard *pb)   {
  char *buf;

  buf       = pb->bufout + pb->bufptr;
  *buf      = i;
  pb->bufptr ++;
  if (pb->bufptr > pb->bufsize - BUFFER_GUARD) scrc_fflush (pb);
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_begin_pasteboard_update (Pasteboard *pb)   {
  if (!pb) return 0;

  if (!pb->updating)
  {
    Update *u;
    int size;

    u = pb->upd;
    size = pb->rows * pb->cols;
    memset (u->map,  ' ', size);
    memset (u->attr, 0, size);
    memset (u->mod_rows, 0, pb->rows);
  }

  pb->updating++;
  return 1;
}

/*---------------------------------------------------------------------------*/
int scrc_end_pasteboard_update (Pasteboard *pb)   {
  char *m, *a, *last;
  char *old_m, *old_a;
  int r, c;
  int cols;
  Update *u;
  char *l;

  if (!pb) return 0;
  /*
  if (pb->updating == 0)
  {
  cursor (pb, pb->curs.row, pb->curs.col);
  scrc_fflush(pb);
  return 0;
  }
  */
  pb->updating--;
  if (pb->updating > 0) return 1;
  pb->updating = 0;

  u = pb->old;
  old_m = u->map;
  old_a = u->attr;

  u = pb->upd;
  m = u->map;
  a = u->attr;
  l = u->mod_rows;

  cols = pb->cols;
  last = a + pb->rows*cols - 1;

  for (r=1; r<=pb->rows; r++, l++)
  {
    if (*l)
    {
      for (c = 1; c <= cols; c++)
      {
        if (*a)
        {
          *a &= ~MODIFIED;
          if (*a != *old_a || *m != *old_m)
          {
            *old_m = *m;
            *old_a = *a;
            scrc_draw_char_on_pb (pb, *m, *a, r, c);
          }
        }

        m++;
        a++;

        old_m++;
        old_a++;
      }
    }
    else
    {
      m += cols;
      a += cols;

      old_m += cols;
      old_a += cols;
    }
  }
  cursor (pb, pb->curs.row, pb->curs.col);
  scrc_fflush(pb);
  return 0;
}

/*----------------------------------------------------------------------------*/
int scrc_count_unmodified (char* attr, char* last)    {
  int count;
  for (count = 0; attr<=last; (attr++, count++))
  {
    if (*attr & MODIFIED) break;
  }
  if (count < 7) for (--attr; count; attr--, count--) *attr |= MODIFIED;
  else count--;
  return count;
}

/*----------------------------------------------------------------------------*/
int cursor (Pasteboard *pb, int row, int col)   {
  char *buf;

  buf = pb->bufout + pb->bufptr;
  *buf = ESCAPE;
  pb->bufptr++;

  buf = pb->bufout + pb->bufptr;
  sprintf (buf, "[%d;%dH", row, col);
  pb->bufptr += strlen(buf);
  if (pb->bufptr > pb->bufsize - BUFFER_GUARD) scrc_fflush (pb);
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_set_scroll (Pasteboard *pb, int row1, int row2)    {
  char *buf;

  buf = pb->bufout + pb->bufptr;
  *buf = ESCAPE;
  pb->bufptr++;

  buf = pb->bufout + pb->bufptr;
  sprintf (buf, "[%d;%dr", row1, row2);
  pb->bufptr += strlen(buf);
  if (pb->bufptr > pb->bufsize - BUFFER_GUARD) scrc_fflush (pb);
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_cursor_on (Pasteboard *pb)   {
  cursor_on();
  return 1;
}

/*----------------------------------------------------------------------------*/
int scrc_cursor_off (Pasteboard *pb)    {
  cursor_off();
  return 1;
}
