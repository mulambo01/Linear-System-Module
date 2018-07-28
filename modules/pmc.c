#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include "matrix.h"

typedef struct
{
 int qtneurons;
 mtx *w;
}pmclayer;

typedef struct
{
 int qtlayers;
 pmclayer *layer ;
}pmcnet;

//types of functions
//sigmoid function
#define SIGM 0
//hyberbolic tangent
#define TGHYP 1
//linear function
#define LINEAR 2

//return the sigmoid function
long double sigm(long double u){
 return 1.0/(1.0+expl(-u));
}
//return the derivative of sigmoid function
long double sigm1(long double u){
 return (1.0-powl(sigm(u),2));
}
long double tghyp(long double u){
 return (tanh(u));
}

long double tghyp1(long double u){
 return (1.0/(powl(cosh(u),2)));
}

long double linear(long double u){
 return (u);
}

long double linear1(long double u){
 return (1.0);
}

//return the value of function of type "ftype"
long double func(long double u, int ftype){
 if(ftype==SIGM)return sigm(u);
 else if(ftype==TGHYP)return tghyp(u);
 else if(ftype==LINEAR)return linear(u);
}
//return the value of derivative function of type "ftype"
long double func1(long double u, int ftype){
 if(ftype==SIGM)return sigm1(u);
 else if(ftype==TGHYP)return tghyp1(u);
 else if(ftype==LINEAR)return linear1(u);
}

pmclayer pmccreatelayer(int qtneurons, int qtw){
 mtx *w=(mtx *)malloc(qtneurons*sizeof(mtx));
 pmclayer layer;
 for(int i=0; i<qtneurons; i++){
  w[i]=randmatrix(1, qtw);
 }
 layer.qtneurons=qtneurons;
 layer.w=w;
 return layer;
}

pmcnet pmccreatenet(int *qtneurons, int qtlayers, int qtw1){
 int qtw;
 pmcnet net;
 pmclayer *layer=(pmclayer *)malloc(qtlayers*sizeof(pmclayer));
 qtw=qtw1;
 for(int i=0; i<qtlayers; i++){
  layer[i]=pmccreatelayer(qtneurons[i], qtw);
  qtw=qtneurons[i]+1; //+1 to include bias
 }
 net.layer=layer;
 net.qtlayers=qtlayers;
 return net;
}

//return the linear combination with values of the synaptic weights and inputs
long double neuronthink(mtx x, mtx w){
 long double input, y;
 input=vectprod(x,w);
 return input;
}
//return the output of a neuron
//input is the answer of function neuronthink
long double neuronansw(long double input, int ftype){
 return func(input, ftype);
}
//return the linear combinations of each neuron with the sample x
mtx layerthink(mtx x, pmclayer layer){
 mtx input;
 int qtneurons=layer.qtneurons;
 input=nullmatrix(1, qtneurons);
 for(int i=0; i<qtneurons; i++){
  input.data[0][i]=neuronthink(x, layer.w[i]);
 }
 return input;
}
//return the output of a neural layer
//input is the answer of function layerthink
mtx layeransw(mtx input, int ftype){
 mtx y;
 y=nullmatrix(1, input.ncols);
 for(int i=0; i<input.ncols; i++){
  y.data[0][i]=neuronansw(input.data[0][i], ftype);
 }
 return y;
}

//return the answer of a neural network
//array w stores the synaptic weights matrix indexing by the layer and position of neuron
//array qtneurons stores the quantity of neurons in each layer
//qtlayers is the quantity of layers
//ftype is the type of activation function
mtx netthink(mtx x, pmcnet net, int *ftype){
 mtx y, input, x0, copy;
 int qtlayers=net.qtlayers;
 x0=crystalmatrix(1,1,-1.0);
 input=mtxclone(x);
 y=nullmatrix(1,1);
 for(int i=0; i<qtlayers; i++){
  copy=layerthink(input, net.layer[i]);
  mtxcopy(&input, copy);
  mtxfree(&copy);
  copy=layeransw(input, ftype[i]);
  mtxcopy(&y, copy);
  mtxfree(&copy);
  mtxcopy(&input, y);
  addcol(&input, x0, 0);
 }
 mtxfree(&input);
 mtxfree(&x0);
 return y;
}

//just call the function netthink and process the output returning the sample class
mtx netansw(mtx x, pmcnet net, int *ftype){
 mtx answ, y;
 y=netthink(x, net, ftype);
 answ=mtxclone(y);
 for(int i=0; i<answ.ncols; i++){
  if(answ.data[0][i]>0.5){
   answ.data[0][i]=1.0;
  }
  else{
   answ.data[0][i]=0.0;
  }
 }
 mtxfree(&y);
 return answ;
}

//it will adjust each weight of a network using the inputs and outputs
void fitbydelta(mtx x, pmcnet net, mtx d, mtx *input, mtx *y, long double lrn, int *ftype){
 mtx delta, lastdelta, change, copy;
 int qtneurons, qtnfrwrd, qtlayers, i, j, layer;
 long double der, del;
 qtlayers=net.qtlayers;
//last layer
 layer=net.qtlayers-1;
 qtneurons=input[layer].ncols;
 change=nullmatrix(1,1);
 delta=nullmatrix(1,qtneurons);
 for(i=0; i<qtneurons; i++){
  der=func1(input[layer].data[0][i], ftype[layer]);
  del=(d.data[0][i]-y[layer].data[0][i])*der;
  delta.data[0][i]=del;
  copy=mtxmult(y[layer-1], del*lrn);
  mtxcopy(&change,copy);
  mtxfree(&copy);
  copy=mtxsum(net.layer[layer].w[i],change);
  mtxcopy(&net.layer[layer].w[i],copy);
  mtxfree(&copy);
 }
 lastdelta=delta;
//middle layers
 for(layer--;layer>0;layer--){
  qtnfrwrd=qtneurons;
  qtneurons=input[layer].ncols;
  delta=nullmatrix(1,qtneurons);
  for(i=0; i<qtneurons; i++){
   der=func1(input[layer].data[0][i], ftype[layer]);
   del=0.0;
   for(j=0; j<qtnfrwrd; j++){
    del=del+lastdelta.data[0][j]*net.layer[layer+1].w[j].data[0][i];
   }
   del=-del*der;
   delta.data[0][i]=del;
   copy=mtxmult(y[layer-1], del*lrn);
   mtxcopy(&change, copy);
   mtxfree(&copy);
   copy=mtxsum(net.layer[layer].w[i], change);
   mtxcopy(&net.layer[layer].w[i], copy);
   mtxfree(&copy);
  }
  mtxfree(&lastdelta);
  lastdelta=delta;
 }
//first layer
 
 qtnfrwrd=qtneurons;
 qtneurons=input[layer].ncols;
 for(i=0; i<qtneurons; i++){
  der=func1(input[layer].data[0][i], ftype[layer]);
  del=0.0;
  for(j=0; j<qtnfrwrd; j++){
   del=del+lastdelta.data[0][j]*net.layer[layer+1].w[j].data[0][i];
  }
  del=-del*der;
  copy=mtxmult(x, del*lrn);
  mtxcopy(&change, copy);
  mtxfree(&copy);
  copy=mtxsum(net.layer[layer].w[i],change);
  mtxcopy(&net.layer[layer].w[i], copy);
  mtxfree(&copy);
 }
 mtxfree(&change);
 mtxfree(&delta);//delta and lastdelta are with the same address
//end
}

//it will genearate the arrays input and y and pass them to the function fitbydelta
void adjust(mtx x, pmcnet net, mtx d, long double lrn, int *ftype){
 mtx *y, *input, yy, ii, x0, copy;
 int i, qtlayers=net.qtlayers;
 x0=crystalmatrix(1,1,-1.0);
 ii=mtxclone(x);
 yy=nullmatrix(1,1);
 y=(mtx *)malloc(qtlayers*sizeof(mtx));
 input=(mtx *)malloc(qtlayers*sizeof(mtx));
 for(i=0; i<qtlayers; i++){
  copy=layerthink(ii, net.layer[i]);
  mtxcopy(&ii, copy);
  mtxfree(&copy);
  copy=layeransw(ii, ftype[i]);
  mtxcopy(&yy, copy);
  mtxfree(&copy);
  input[i]=mtxclone(ii);
  if(i!=qtlayers-1){
   addcol(&yy, x0, 0);
  }
  y[i]=mtxclone(yy);
  mtxcopy(&ii, yy);
 }
 fitbydelta(x, net, d, input, y, lrn, ftype);
 mtxfree(&yy);
 mtxfree(&ii);
 mtxfree(&x0);
 for(i=0; i<qtlayers; i++){
  mtxfree(&y[i]);
  mtxfree(&input[i]);
 }
 free(y);
 free(input);
}

void adjustbymomentum(pmcnet *net, pmcnet *oldnet1, pmcnet *oldnet2, long double momentum){
 mtx var, copy;
 long double **wchange;
 var=nullmatrix(1,1);
 int j, qtlayers;
 qtlayers=net->qtlayers;
 for(int i=0; i<qtlayers; i++){
  for(j=0; j<net->layer[i].qtneurons; j++){
   copy=mtxsub(oldnet1->layer[i].w[j], oldnet2->layer[i].w[j]);
   mtxcopy(&var, copy);
   mtxfree(&copy);
   copy=mtxmult(var, momentum);
   mtxcopy(&var, copy);
   mtxfree(&copy);
   copy=mtxsum(net->layer[i].w[j], var);
   mtxcopy(&(net->layer[i].w[j]),copy);
   mtxfree(&copy);
   wchange=oldnet2->layer[i].w[j].data;
   oldnet2->layer[i].w[j].data=oldnet1->layer[i].w[j].data;
   oldnet1->layer[i].w[j].data=wchange;
   mtxcopy(&(oldnet1->layer[i].w[j]), net->layer[i].w[j]);
  }
 }
 mtxfree(&var);
}

//calculate the mean square error of the samples
long double meansqrerr(mtx samples, pmcnet net, mtx d, int *ftype){
 long double result=0.0;
 int qtspl=samples.nrows;
 int qtinput=samples.ncols;
 int qtlayers;
 mtx u, x, output, error, copy;
 u=nullmatrix(d.nrows, d.ncols);
 x=nullmatrix(1, qtinput);
 output=nullmatrix(1, d.ncols);
 for(int i=0; i<qtspl; i++){
  copy=mtxcut(samples, i, 1, 0, qtinput);
  mtxcopy(&x,copy);
  mtxfree(&copy);
  copy=netthink(x, net, ftype);
  mtxcopy(&output, copy);
  mtxfree(&copy);
  putline(&u, output, i);
 }
 copy=mtxmult(u, -1.0);
 mtxcopy(&u, copy);
 mtxfree(&copy);
 error=mtxsum(u, d);
 result=vectprod(error, error)/qtspl;
 mtxfree(&u);
 mtxfree(&x);
 mtxfree(&output);
 mtxfree(&error);

 return result;
}

//save a layer in a text file
//receive the array of weights by neuron, put it in a matrix and save it
void savelayer(char *filename, pmclayer layer){
 mtx wbylayer;
 int qtneurons=layer.qtneurons;
 wbylayer=nullmatrix(qtneurons, layer.w[0].ncols);
 for(int i=0; i<qtneurons; i++){
  putline(&wbylayer, layer.w[i], i);
 }
 mtxsave(filename, wbylayer);
 mtxfree(&wbylayer);
}

//save a network in a text file
//dirname is the name of directory
//fileprefix is the the prefix that each layer file will receive
void savenet(char *dirname, char *fileprefix, pmcnet net){
 int namelen, sufixlen, qtlayers;
 qtlayers=net.qtlayers;
 sufixlen=1+(int)log10(qtlayers-1);
 namelen=strlen(dirname)+strlen(fileprefix)+sufixlen+10;
 char filename[namelen], sufix[sufixlen];
 mkdir(dirname, S_IRWXU);
 for(int i=0; i<qtlayers; i++){
  sprintf(sufix, "%d", (i+(int)pow(10.0,sufixlen)));
  sufix[0]='-';
  sprintf(filename, "%s/%s%s.dat",dirname, fileprefix, sufix);
  savelayer(filename, net.layer[i]);
 }
}

//load a neural layer by a text file
pmclayer loadlayer(char *filename, int qtneurons, int qtw){
 mtx fileload, *w;
 pmclayer layer;
 w=(mtx *)malloc(qtneurons*sizeof(mtx));
 fileload=mtxload(filename, qtneurons, qtw);
 for(int i=0; i<qtneurons; i++){
  w[i]=mtxcut(fileload, i, 1, 0, qtw);
 }
 layer.w=w;
 layer.qtneurons=qtneurons;
 mtxfree(&fileload);
 return layer;
}

//load a neural network by the files of a directory with a prefix
pmcnet loadnet(char *dirname, char *fileprefix, int *qtneurons, int qtlayers, int qtw1){
 pmclayer *layer=(pmclayer *)malloc(qtlayers*sizeof(layer));
 pmcnet net;
 int namelen, sufixlen, qtw;
 sufixlen=1+(int)log10(qtlayers-1);
 namelen=strlen(dirname)+strlen(fileprefix)+sufixlen+10;
 char filename[namelen], sufix[sufixlen];
 qtw=qtw1;
 for(int i=0; i<qtlayers; i++){
  sprintf(sufix, "%d", (i+(int)pow(10.0,sufixlen)));
  sufix[0]='-';
  sprintf(filename, "%s/%s%s.dat",dirname, fileprefix, sufix);
  layer[i]=loadlayer(filename, qtneurons[i], qtw);
  qtw=qtneurons[i]+1;
 }
 net.layer=layer;
 net.qtlayers=qtlayers;
 return net;
}

pmclayer clonelayer(pmclayer layer){
 pmclayer clone;
 int qtneurons=layer.qtneurons;
 mtx *w=(mtx *)malloc(qtneurons*sizeof(mtx));
 for(int i=0; i<qtneurons; i++){
  w[i]=mtxclone(layer.w[i]);
 }
 clone.qtneurons=qtneurons;
 clone.w=w;
 return clone;
}

pmcnet clonenet(pmcnet net){
 pmcnet clone;
 pmclayer *layer;
 int qtlayers=net.qtlayers;
 layer=(pmclayer *)malloc(qtlayers*sizeof(pmclayer));
 for(int i=0; i<qtlayers; i++){
  layer[i]=clonelayer(net.layer[i]);
 }
 clone.qtlayers=qtlayers;
 clone.layer=layer;
 return clone;
}
