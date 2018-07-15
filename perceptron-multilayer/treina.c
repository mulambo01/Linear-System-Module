#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../modules/pmc.h"


void main(){
 mtx samples, ds, **w, output, sample, d;
 char filename[100];
 int i, j, qtspl, qtdatabyrow, *qtneurons, qtinput, qtlayers, steps, ftype;
 long double Eqm, lastEqm, precis, lrn;
 ftype=0;
 precis=10.e-6;
 lrn=0.1;
 qtspl=130;
 qtlayers=2;
 qtneurons=(int *)malloc(qtlayers*sizeof(int));
 qtinput=5;//4 +1
 qtneurons[0]=10;
 qtneurons[1]=3;
 qtdatabyrow=qtinput+qtneurons[1];
 samples=mtxload("Tabela3.txt", qtspl, qtdatabyrow);
 w=createneurons(qtneurons, qtlayers, qtinput);
 ds=mtxcut(samples,0, qtspl, qtinput, qtdatabyrow-qtinput);
 samples=mtxcut(samples, 0, qtspl, 0, qtinput);

 d=nullmatrix(1,1);
 sample=nullmatrix(1,1);
 steps=0;
 lastEqm=0.0;
 Eqm=meansqrerr(samples, ds, w, qtneurons, qtlayers, ftype);
 while(fabsl(Eqm-lastEqm)>precis){
  lastEqm=Eqm;
  for(i=0; i<samples.nrows; i++){
   mtxcopy(&sample,mtxcut(samples, i, 1, 0, qtinput));
   mtxcopy(&d,mtxcut(ds, i, 1, 0, ds.ncols));
   adjust(sample, d, w, qtneurons, qtlayers, lrn, ftype);
  }
  Eqm=meansqrerr(samples, ds, w, qtneurons, qtlayers, ftype);
  steps++;
 }
savenet("layers", "layer",w, qtneurons, qtlayers);
}
