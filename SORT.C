void quicksort(int l,int r,int type)
{
  register int i,j;
  uploadsrec a,a2,x;

  i=l; j=r;
  SETREC(((l+r)/2));
  read(dlf, (void *)&x,sizeof(uploadsrec));
  do {
    SETREC(i);
    read(dlf, (void *)&a,sizeof(uploadsrec));
    while (comparedl(&a,&x,type)<0) {
      SETREC(++i);
      read(dlf, (void *)&a,sizeof(uploadsrec));
    }
    SETREC(j);
    read(dlf, (void *)&a2,sizeof(uploadsrec));
    while (comparedl(&a2,&x,type)>0) {
      SETREC(--j);
      read(dlf, (void *)&a2,sizeof(uploadsrec));
    }
    if (i<=j) {
      if (i!=j) {
        SETREC(i);
        write(dlf,(void *)&a2,sizeof(uploadsrec));
        SETREC(j);
        write(dlf,(void *)&a,sizeof(uploadsrec));
      }
      i++;
      j--;
    }
  } while (i<j);
  if (l<j)
    quicksort(l,j,type);
  if (i<r)
    quicksort(i,r,type);
}


void sortdir(int dn, int type)
{
  dliscan1(dn);
  if (numf>1)
    quicksort(1,numf,type);
  closedl();
}


void sort_all(int type)
{
  int i;

  for (i=0; (i<64) && (udir[i].subnum!=-1) && (!kbhitb()); i++) {
    nl();
    ansic(1);
    print("4Sorting");
    sortdir(i,type);
  }
}
