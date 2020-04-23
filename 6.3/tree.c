//
//  
//  
//
//  Created by William McCarthy on 134//20.
//  Copyright © 2020 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "tree.h"
#include "utils.h"


inode* inode_create(int line_num){
    inode* p = (inode*)malloc(sizeof(inode));
    p->line_num = line_num;
    p->next = NULL;

    return p;
}

void inode_append(inode* head, int line_num){
    inode* p = head;
    while(p->next != NULL){
        p = p->next;
    }
    if(p->line_num != line_num){
        inode* q = inode_create(line_num);
        p->next = q;
    }
}

void inode_delete(inode* p){
    while(p != NULL){
        inode* q = p;
        p = p->next;
        free(q);
    }
}
//-------------------------------------------------
tnode* tnode_create(const char* word, int line_num) {
  tnode* p = (tnode*)malloc(sizeof(tnode));
  p->word = strdup(word);    // copy of word allocated on heap
  p->count = 1;
  p->line_node = inode_create(line_num);
  p->left = NULL;
  p->right = NULL;
  
  return p;
}

//====================================================================
void tnode_delete(tnode* p) {
    free((void*)p->word);
    free(p);
}

//====================================================================
tree* tree_create(void) {
  tree* p = (tree*)malloc(sizeof(tree));
  p->root = NULL;
  p->size = 0;
  
  return p;
}

//====================================================================
static void tree_deletenodes(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_deletenodes(t, p->left);
  tree_deletenodes(t, p->right);
  tnode_delete(p);
  t->size--;
}

//====================================================================
void tree_clear(tree* t) {
  tree_delete(t);
  t->root = NULL;
  t->size = 0;
}

//====================================================================
void tree_delete(tree* t) { tree_deletenodes(t, t->root); }

//====================================================================
bool tree_empty(tree* t)  { return t->size == 0; }

//====================================================================
size_t tree_size(tree* t) { return t->size; }

//====================================================================
static tnode* tree_addnode(tree* t, tnode** p, const char* word, int line_num) {
  int compare;
  
  if (*p == NULL) {
    *p = tnode_create(word, line_num);
  } else if ((compare = strcmp(word, (*p)->word)) == 0) {
    (*p)->count++;
    inode_append((*p)->line_node, line_num);
  } else if (compare < 0) { tree_addnode(t, &(*p)->left, word,line_num);
  } else {
    tree_addnode(t, &(*p)->right, word, line_num);
  }

  return *p;
}

//====================================================================
static char* str_process(char* s, char* t) {
  char* p = s;
  char ignore[] = "\'\".,;;?!()/’";
  while (*t != '\0') {
    if (strchr(ignore, *t) == NULL || (*t == '\'' && (p != s || p != s + strlen(s) - 1))) {
      *p++ = tolower(*t);
    }
    ++t;
  }
  *p++ = '\0';
  return s;
}

//====================================================================
tnode* tree_add(tree* t, char* word, int line_num) {
  char buf[100];
  
  if (word == NULL) { return NULL; }
  str_process(buf, word);
  
  tnode* p = tree_addnode(t, &(t->root), buf, line_num);
  t->size++;

  return p;
}

//====================================================================
static void tree_printme(tree* t, tnode* p) {
    inode* q = p->line_node;
  if (p->count > 1) { printf("%5d -- ", p->count); }
  else {
    printf("         ");
  }
  printf("%-18s -- {%d", p->word, q->line_num);
  while((q = q->next) != NULL){
      printf(", %d", q->line_num);
  }
  printf("}\n");
}

//====================================================================
static void tree_printnodes(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printnodes(t, p->left);
  tree_printme(t, p);
  tree_printnodes(t, p->right);
}

//====================================================================
static void tree_printnodes_preorder(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printme(t, p);
  tree_printnodes(t, p->left);
  tree_printnodes(t, p->right);
}

//====================================================================
static void tree_printnodes_postorder(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printnodes_postorder(t, p->left);
  tree_printnodes_postorder(t, p->right);
  tree_printme(t, p);
}

//====================================================================
static void tree_printnodes_reverseorder(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printnodes_reverseorder(t, p->right);
  tree_printme(t, p);
  tree_printnodes_reverseorder(t, p->left);
}

//====================================================================
void tree_print(tree* t)              { tree_printnodes(t, t->root);               printf("\n"); }

//====================================================================
void tree_print_preorder(tree* t)     { tree_printnodes_preorder(t, t->root);      printf("\n"); }

//====================================================================
void tree_print_postorder(tree* t)    { tree_printnodes_postorder(t, t->root);     printf("\n"); }

//====================================================================
void tree_print_reverseorder(tree* t) { tree_printnodes_reverseorder(t, t->root);  printf("\n"); }

//====================================================================
void tree_test(tree* t) {
  printf("=============== TREE TEST =================================\n");
  printf("\n\nprinting in order...========================================\n");
  tree_print(t);
  printf("end of printing in order...=====================================\n\n");

  printf("\n\nprinting in reverse order...================================\n");
  tree_print_reverseorder(t);
  printf("end of printing in reverse order...=============================\n\n");
  printf("tree size is: %zu\n", tree_size(t));
  
  printf("clearing tree...\n");
  tree_clear(t);
  printf("after clearing tree, size is: %zu\n", tree_size(t));
  tree_print(t);
  
  printf("=============== END OF TREE TEST ==========================\n");
}

//====================================================================
tree* tree_from_file(int argc, const char* argv[]) {
  if (argc != 2) { return NULL; }

  FILE* fin;
  const char* filename = argv[1];
  if ((fin = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Could not open file: '%s'\n", filename);
    exit(1);
  }

  char buf[BUFSIZ];
  char delims[] = " \n";
  int size = 0;
  int line_num = 0;
  memset(buf, 0, sizeof(buf));

  tree* t = tree_create();
  while (fgets(buf, BUFSIZ, fin)) {
    line_num++;
    char* word = strtok(buf, delims);
    tree_add(t, word, line_num);

    while ((word = strtok(NULL, delims)) != NULL) {
      tree_add(t, word, line_num);
    }
  }
  printf("%d words added...\n", size);
  fclose(fin);

  return t;
}


//====================================================================
int main(int argc, const char* argv[]) {
  tree* t = tree_from_file(argc, argv);
  if (t != NULL) { tree_test(t);  tree_delete(t);  return 0; }
  
  char buf[BUFSIZ];
  int line_num = 0;
  char delims[] = " .,;?!\'\"()\n";
  int size = 0;
  memset(buf, 0, sizeof(buf));
  

  t = tree_create();
  while (fgets(buf, BUFSIZ, stdin)) {
    line_num++;
    char* word = strtok(buf, delims);
    tree_add(t, word, line_num);

    while ((word = strtok(NULL, delims)) != NULL) {
      tree_add(t, word, line_num);
    }
  }
  printf("%d words added...\n", size);

  tree_test(t);
  tree_delete(t);
  
  return 0;
}

