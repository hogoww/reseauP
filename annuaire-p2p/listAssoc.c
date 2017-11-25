struct list{
  char* v;
  struct list* next;
};


struct list* make_list(char* val){
  struct list* res=malloc(sizeof(struct list));
  res->v=val;
  res->next=NULL;
  return res;
}

void delete_list(struct list* l){/*free only "next"s pointers*/
  if(l->next!=NULL)
    delete_list(l->next);
  free(l);
}

void delete_list_and_values(struct list* l){
 if(l->next!=NULL)
    delete_list_and_values(l->next);
 free(l->v);
 free(l);
}

struct list* add_value_list(struct list *l,char* v){/*to be able to chain those operations*/
  while(l->next!=NULL){
    l=l->next;
  }

  l->next=malloc(sizeof(struct list));
  l->next->val=v;
  l->next->next=NULL;
  
  return l;
}



struct listAssoc{
  char* k;
  struct list l;
  struct listAssoc next;
};

  

struct listAssoc* make_ListAssoc(char* key){
  struct listAssoc* res=malloc(sizeof(struct listAssoc));
  res->k=key;
  res->l=NULL;
  res->next=NULL;
  return res;
}

void delete_listAssoc(struct listAssoc* l){/*free only "next"s pointers*/
  if(l->next!=NULL)
    delete_listAssoc(l->next);
  if(l->l==NULL){
    delete_list(l->l);
  }
  free(l);
}

void delete_listAssoc_and_key_and_values(struct listAssoc* l){
  if(l->next!=NULL)
    delete_listAssoc(l->next);
  if(l->l==NULL){
    delete_list_and_value(l->l);
  }
  free(l->k);
  free(l);
}

struct listAssoc* get_key_listAssoc(struct listAssoc* l,char *key){
  if(l==NULL){
    return NULL;
  }
  else{
    if(!strcmp(key,l->key))
       return l;
    else{
      if(l->next==NULL){
	l->next=make_ListAssoc(key);
	return l->next;
      }
      else{
	return get_key_listAssoc(l->next,key);
      }
    }
  }
}

struct listAssoc* addValue_to_key_list(char* key,char* value){
  l->l.
}
