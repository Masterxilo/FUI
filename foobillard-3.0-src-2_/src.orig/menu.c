#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/gl.h>
#include "options.h"
#include "sys_stuff.h"


menuType * menu_new( void (* callback)( int, void * ) )
{
    menuType * menu;

    DPRINTF("menu_new:\n");

    menu = malloc(sizeof(menuType));
    menu->nr           = 0;
    menu->select_id    = -1;
    menu->p_select_id  = &(menu->select_id);
    menu->select_index = 0;
    menu->textedit_mode = 0;
    menu->callback     = callback;
    menu->parent       = (menuType *)0;
    return menu;
}


void menu_add_submenu( menuType * menu, char * text, menuType * submenu, int show_subsetting )
{
    DPRINTF("menu_add_submenu:\n");
    if(menu->nr<MAX_MENU_ENTRY_NUM){
        menu->entry[menu->nr].type     = ENTRY_TYPE_SUBMENU;
        strcpy( menu->entry[menu->nr].text, text);
        menu->entry[menu->nr].settingtext[0] = 0;
        menu->entry[menu->nr].submenu  = submenu;
        menu->entry[menu->nr].text_obj = (textObj *)0;
//    menu->entry[menu->nr].settingtext_obj = (textObj *)0;
        menu->entry[menu->nr].fontname = options_menu_fontname;
        menu->entry[menu->nr].fontsize = 32;
        menu->entry[menu->nr].show_subsetting = show_subsetting;
        menu->entry[menu->nr].arg      = (void *) 0;
        if(submenu!=(menuType *)0){
            submenu->callback     = menu->callback;
            submenu->p_select_id  = menu->p_select_id;
            submenu->parent       = menu;
            submenu->parent_entry = &(menu->entry[menu->nr]);
        }
        menu->nr++;
    } else {
        printf("menu_add_submenu: too many menu entries - ignoring this entry\n");
    }
}


void menu_add_entry( menuType * menu, char * text, int id )
{
    DPRINTF("menu_add_entry:\n");
    if(menu->nr<MAX_MENU_ENTRY_NUM){
        menu->entry[menu->nr].type     = ENTRY_TYPE_ID;
        strcpy( menu->entry[menu->nr].text, text );
        menu->entry[menu->nr].settingtext[0] = 0;
        menu->entry[menu->nr].id       = id;
        menu->entry[menu->nr].text_obj = (textObj *)0;
//    menu->entry[menu->nr].settingtext_obj = (textObj *)0;
        menu->entry[menu->nr].fontname = options_menu_fontname;
        menu->entry[menu->nr].fontsize = 32;
        menu->entry[menu->nr].arg      = (void *) 0;
        menu->nr++;
    } else {
        printf("menu_add_entry: too many menu entries - ignoring this entry\n");
    }
}


void menu_add_arg_entry( menuType * menu, char * text, int id, void * arg )
{
    DPRINTF("menu_add_arg_entry:\n");
    if(menu->nr<MAX_MENU_ENTRY_NUM){
        menu->entry[menu->nr].type     = ENTRY_TYPE_ID;
        strcpy( menu->entry[menu->nr].text, text );
        menu->entry[menu->nr].settingtext[0] = 0;
        menu->entry[menu->nr].id       = id;
        menu->entry[menu->nr].text_obj = (textObj *)0;
        //    menu->entry[menu->nr].settingtext_obj = (textObj *)0;
        menu->entry[menu->nr].fontname = options_menu_fontname;
        menu->entry[menu->nr].fontsize = 32;
        menu->entry[menu->nr].arg      = arg;
        menu->nr++;
    } else {
        printf("menu_add_arg_entry: too many menu entries - ignoring this entry\n");
    }
}


void menu_add_textfield( menuType * menu, char * text, int id, int fixedlen )
{
    DPRINTF("menu_add_arg_entry:\n");
    if(menu->nr<MAX_MENU_ENTRY_NUM){
        menu->entry[menu->nr].type     = ENTRY_TYPE_TEXTFIELD;
        strcpy( menu->entry[menu->nr].text, text );
        menu->entry[menu->nr].settingtext[0] = 0;
        menu->entry[menu->nr].id       = id;
        menu->entry[menu->nr].text_obj = (textObj *)0;
        //    menu->entry[menu->nr].settingtext_obj = (textObj *)0;
        menu->entry[menu->nr].fontname = options_menu_fontname;
        menu->entry[menu->nr].fontsize = 32;
        menu->entry[menu->nr].arg      = (void *)0;
        menu->entry[menu->nr].fixedlen = fixedlen;
        menu->nr++;
    } else {
        printf("menu_add_textfield: too many menu entries - ignoring this entry\n");
    }
}


void menu_add_exit( menuType * menu, char * text )
{
    DPRINTF("menu_add_exit:\n");
    if(menu->nr<MAX_MENU_ENTRY_NUM){
        menu->entry[menu->nr].type     = ENTRY_TYPE_EXIT;
        strcpy( menu->entry[menu->nr].text, text );
        menu->entry[menu->nr].settingtext[0] = 0;
        menu->entry[menu->nr].text_obj = (textObj *)0;
//    menu->entry[menu->nr].settingtext_obj = (textObj *)0;
        menu->entry[menu->nr].fontname = options_menu_fontname;
        menu->entry[menu->nr].fontsize = 32;
        menu->entry[menu->nr].arg      = (void *) 0;
        menu->nr++;
    } else {
        printf("menu_add_exit: too many menu entries - ignoring this entry\n");
    }
}


void menu_entry_set_text( menuEntry * entry, char * text )
{
    char str[256];
    strcpy( entry->text, text );
    if( entry->text_obj != (textObj *)0 ){
        if( entry->show_subsetting && entry->settingtext[0]!=0 ){
            sprintf(str,"%s : %s",entry->text,entry->settingtext);
            textObj_setText( entry->text_obj, str );
//            textObj_setText( entry->text_obj, text );
        } else {
            textObj_setText( entry->text_obj, text );
        }
    }
}


void menu_entry_set_settingtext( menuEntry * entry, char * text )
{
    char str[256];
    strcpy( entry->settingtext, text );
    if( entry->text_obj != (textObj *)0 ){
        if( entry->show_subsetting && entry->settingtext[0]!=0 ){
            sprintf(str,"%s : %s",entry->text,entry->settingtext);
            textObj_setText( entry->text_obj, str );
/*            sprintf(str," : %s",entry->settingtext);
            if( entry->settingtext_obj != (textObj *)0 ){
                textObj_setText( entry->settingtext_obj, str );
            } else {
                entry->settingtext_obj=textObj_new( str, entry->fontname, entry->fontsize );
            }*/
        } else {
            textObj_setText( entry->text_obj, text );
        }
    }
}


static void menu_create_textobj( menuEntry * entry )
{
    char str[256];
    if( entry->text_obj == (textObj *)0 ){
        if( entry->show_subsetting && entry->settingtext[0]!=0 ){
            sprintf(str,"%s : %s",entry->text,entry->settingtext);
            entry->text_obj = textObj_new( str, entry->fontname, entry->fontsize );
/*            entry->text_obj = textObj_new( entry->text, entry->fontname, entry->fontsize );
            sprintf(str," : %s",entry->settingtext);
            entry->settingtext_obj = textObj_new( str, entry->fontname, entry->fontsize );*/
        } else {
            entry->text_obj = textObj_new( entry->text, entry->fontname, entry->fontsize );
        }
//        textObj_toggle3D(entry->text_obj);
    }
}

void menu_select_by_coord( menuType * menu, int x, int y )
/* selects the menupoint under the pos x,y */
{
    int i,x1,y1,x2,y2, all_height;

    if(!menu->textedit_mode){
        all_height=0;
        for(i=0;i<menu->nr;i++){
            all_height+=menu->entry[i].fontsize;
        }

        for(i=0;i<menu->nr;i++){
            //        fprintf( stderr, "menu->entry[i].text_obj = %d\n", menu->entry[i].text_obj );
            if( menu->entry[i].text_obj == (textObj *)0 ){
                menu_create_textobj( &(menu->entry[i]) );
            }
            x1 = -menu->entry[i].text_obj->quad_w/2;
            y1 = all_height/2-i*menu->entry[i].fontsize-menu->entry[i].text_obj->quad_h/2;
            x2 = +menu->entry[i].text_obj->quad_w/2;
            y2 = all_height/2-i*menu->entry[i].fontsize+menu->entry[i].text_obj->quad_h/2;
            if( x<=x2 && x>=x1 && y<=y2 && y>=y1 ){
                menu->select_index=i;
                *(menu->p_select_id)=menu->entry[i].id;
            }
        }
    }
}


void menu_select_next( menuType * menu )
/* selects the next menupoint in the current submenu */
{
    if(!menu->textedit_mode){
        (menu->select_index)++;
        if( menu->select_index >= menu->nr ) menu->select_index=0;
        *(menu->p_select_id) = menu->entry[menu->select_index].id;
    }
}

void menu_select_prev( menuType * menu )
/* selects the previous menupoint in the current submenu */
{
    if(!menu->textedit_mode){
        (menu->select_index)--;
        if( menu->select_index < 0 ) menu->select_index=(menu->nr)-1;
        *(menu->p_select_id) = menu->entry[menu->select_index].id;
    }
}

void menu_choose(menuType ** menu)
{
    DPRINTF("menu_choose:\n");
    if( (*menu)->entry[(*menu)->select_index].type==ENTRY_TYPE_SUBMENU ) {

        DPRINTF("menu_choose: switch submenu\n");
//        menu_texObj_cleanup(menu);
        (*menu)=(*menu)->entry[(*menu)->select_index].submenu;

    } else if( (*menu)->entry[(*menu)->select_index].type==ENTRY_TYPE_ID ) {

        DPRINTF("menu_choose: id=%d\n",(*menu)->entry[(*menu)->select_index].id);
        if( (*menu)->parent_entry != 0 ){
            if( (*menu)->parent_entry->show_subsetting ){
                menu_entry_set_settingtext( (*menu)->parent_entry, (*menu)->entry[(*menu)->select_index].text );
            }
        }
        (*menu)->callback( (*menu)->entry[(*menu)->select_index].id,
                           (*menu)->entry[(*menu)->select_index].arg );
        menu_exit(menu);

    } else if( (*menu)->entry[(*menu)->select_index].type==ENTRY_TYPE_TEXTFIELD ) {
        DPRINTF("menu_choose: ENTRY_TYPE_TEXTFIELD\n");
        if(!(*menu)->textedit_mode){
            DPRINTF("menu_choose: ENTRY_TYPE_TEXTFIELD - !textedit_mode\n");
            (*menu)->textedit_mode=1;
        } else {
            DPRINTF("menu_choose: ENTRY_TYPE_TEXTFIELD - textedit_mode\n");
            (*menu)->textedit_mode=0;
            strcpy((*menu)->entry[(*menu)->select_index].text,
                   (*menu)->entry[(*menu)->select_index].text_obj->str);
            /* give the entered string as arg */
            (*menu)->entry[(*menu)->select_index].arg = (void *)&((*menu)->entry[(*menu)->select_index].text[(*menu)->entry[(*menu)->select_index].fixedlen]);
            DPRINTF("printing arg:\n");
            DPRINTF("%s\n",(char *)((*menu)->entry[(*menu)->select_index].arg));
            (*menu)->callback( (*menu)->entry[(*menu)->select_index].id,
                               (*menu)->entry[(*menu)->select_index].arg );
        }
    } else if( (*menu)->entry[(*menu)->select_index].type==ENTRY_TYPE_EXIT ) {

        menu_exit(menu);

    }
}


void menu_exit(menuType ** menu)
{
    DPRINTF("menu_exit:\n");
    if(!(*menu)->textedit_mode){
/*    if( (*menu)->parent != (menuType *)0 ){*/
        DPRINTF("menu_exit: switch to parent\n");
        menu_texObj_cleanup(*menu);
        *menu=(*menu)->parent;
        DPRINTF("menu_exit: switch to parent - end\n");
/*    } else {
        fprintf(stderr,"menu_exit: cleanup only\n");
        menu_texObj_cleanup(*menu);
        }*/
    } else {
        textObj_setText( (*menu)->entry[(*menu)->select_index].text_obj, (*menu)->entry[(*menu)->select_index].text );
        (*menu)->textedit_mode=0;
    }
}



void menu_text_keystroke( menuType * menu, int key )
{
    if(menu->textedit_mode){
        switch(key){
        case 8:
            if(strlen(menu->entry[menu->select_index].text_obj->str) > menu->entry[menu->select_index].fixedlen)
                textObj_delete_last( menu->entry[menu->select_index].text_obj );
            break;
        default:
            if (isprint(key))
                textObj_append_char( menu->entry[menu->select_index].text_obj, key );
            break;
        }
    }
}



static void menu_draw_entry( menuEntry * entry )
{
    if( entry->text_obj == (textObj *)0 ){
        menu_create_textobj( entry );
    }
    textObj_draw_centered(entry->text_obj);
/*    if( entry->show_subsetting ){
        textObj_draw_bound(entry->text_obj,        HBOUND_RIGHT, VBOUND_CENTER );
        if(entry->settingtext_obj != (textObj *)0 ){
            textObj_draw_bound(entry->settingtext_obj, HBOUND_LEFT,  VBOUND_CENTER );
        }
    } else {
        textObj_draw_bound(entry->text_obj, HBOUND_CENTER, VBOUND_CENTER );
    }*/
}

void menu_draw( menuType * menu )
{
    int i,all_height;

//    fprintf(stderr,"menu_draw:\n");

//    fprintf(stderr,"menu_draw: calc total height\n");
    all_height=0;
    for(i=0;i<menu->nr;i++){
        all_height+=menu->entry[i].fontsize;
    }

//    fprintf(stderr,"menu_draw: draw\n");
    glPushMatrix();
    glTranslatef(0,all_height/2,0);
//    for(i=0;i<1/*menu->nr*/;i++){
    for(i=0;i<menu->nr;i++){
        /* hilight the entry with id=*p_select_id */
//        fprintf(stderr,"menu_draw: draw entry#%d\n",i);
        if( i==menu->select_index ){     /* selected menu entry */
            if(menu->textedit_mode){     /* if textedit-field */
                glColor3f(1.0,0.0,0.0);
                menu_draw_entry(&(menu->entry[i]));
            } else {
                glPushMatrix();
                glScalef(1.1,1.1,1.0);
                glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_COLOR );
                glColor3f(1.0,1.0,1.0);
                glTranslatef(2,2,0);
                menu_draw_entry(&(menu->entry[i]));
                glTranslatef(-4,0,0);
                menu_draw_entry(&(menu->entry[i]));
                glTranslatef(0,-4,0);
                menu_draw_entry(&(menu->entry[i]));
                glTranslatef(4,0,0);
                menu_draw_entry(&(menu->entry[i]));
                glTranslatef(-2,2,0);
                glBlendFunc(GL_ONE,GL_ONE);
                glColor3f(1.0,1.0,0.0);
                menu_draw_entry(&(menu->entry[i]));
                glPopMatrix();
            }
        } else {         /* normal menu entry */
            glPushMatrix();
            glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
            glColor3f(1.0,1.0,1.0);
            glTranslatef(2,2,0);
            menu_draw_entry(&(menu->entry[i]));
            glTranslatef(-4,0,0);
            menu_draw_entry(&(menu->entry[i]));
            glTranslatef(0,-4,0);
            menu_draw_entry(&(menu->entry[i]));
            glTranslatef(4,0,0);
            menu_draw_entry(&(menu->entry[i]));
            glTranslatef(-2,2,0);
            glBlendFunc(GL_ONE,GL_ONE);
            glColor3f(1.0,1.0,1.0);
            menu_draw_entry(&(menu->entry[i]));
            glPopMatrix();
        }

        glTranslatef(0,-menu->entry[i].fontsize,0);
    }
    glPopMatrix();
//    fprintf(stderr,"menu_draw: end\n");
}


void menu_texObj_cleanup(menuType * menu)
/* releases the gl-lists and the allocated textures */
{
    int i;

    DPRINTF("menu_texObj_cleanup:\n");
    if( menu != (menuType *)0 ){
        for ( i=0 ; i<menu->nr ; i++ ) {
            DPRINTF("menu_texObj_cleanup: i=%d (%s)\n",i,menu->entry[i].text);

            if( menu->entry[i].text_obj != (textObj *)0 )
                textObj_delete(menu->entry[i].text_obj);
            DPRINTF("menu_texObj_cleanup: 1\n");
//            if( menu->entry[i].settingtext_obj != (textObj *)0 )
//                textObj_delete(menu->entry[i].settingtext_obj);

            if( menu->entry[i].type == ENTRY_TYPE_SUBMENU ){
                DPRINTF("menu_texObj_cleanup: menu_texObj_cleanup\n");
                menu_texObj_cleanup(menu->entry[i].submenu);
            }
            DPRINTF("menu_texObj_cleanup: 2\n");

            if( menu->entry[i].text_obj != (textObj *)0 )
                free(menu->entry[i].text_obj);

            DPRINTF("menu_texObj_cleanup: 3\n");
            menu->entry[i].text_obj = (textObj *)0;
            DPRINTF("menu_texObj_cleanup: 4\n");
        }
    }
}
