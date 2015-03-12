#ifndef MENU_H
#define MENU_H

#include "myinclude.h"
#include "textobj.h"

#define MAX_MENU_ENTRY_NUM 50

enum entryType {
    ENTRY_TYPE_ID,
    ENTRY_TYPE_SUBMENU,
    ENTRY_TYPE_TEXTFIELD,
    ENTRY_TYPE_EXIT
};


struct menu_entry_struct;
struct menu_struct;

typedef struct menu_entry_struct menuEntry;
typedef struct menu_struct menuType;

/*typedef*/ struct menu_entry_struct{

    char            text[256];
    char            settingtext[256];
    enum entryType  type;
    menuType *      submenu;
    int             id;
    char *          fontname;
    int             fontsize;
    textObj *       text_obj;
//    textObj *       settingtext_obj;
    int             show_subsetting;
    void *          arg;
    int             fixedlen; /* fixed length that may not be changed in textfields */

}/* menuEntry*/;


/*typedef */struct menu_struct{

    menuEntry   entry[MAX_MENU_ENTRY_NUM];
    int         nr;
    void        (* callback)( int, void * );
    int         select_index;
    int         select_id;
    int *       p_select_id;
    char *      fontname;
    int         fontsize;
    int         textedit_mode;
    menuType *  parent;
    menuEntry * parent_entry;

}/* menuType*/;


menuType * menu_new( void (* callback)( int, void * ) );
void menu_add_submenu( menuType * menu, char * text, menuType * submenu, int show_subsetting );
void menu_add_entry( menuType * menu, char * text, int id );
void menu_add_arg_entry( menuType * menu, char * text, int id, void * arg );
void menu_add_textfield( menuType * menu, char * text, int id, int fixedlen );
void menu_add_exit( menuType * menu, char * text );
void menu_select_by_coord( menuType * menu, int x, int y );
void menu_select_next( menuType * menu );
void menu_select_prev( menuType * menu );
void menu_choose(menuType ** menu);
void menu_exit(menuType ** menu);
void menu_text_keystroke( menuType * menu, int key );
void menu_draw( menuType * menu );
void menu_texObj_cleanup(menuType * menu);


#endif /* MENU_H */
