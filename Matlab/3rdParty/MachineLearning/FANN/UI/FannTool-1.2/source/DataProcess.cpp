#include "DataProcess.h"


int GetnItem(char *FileName)
{
  string line;
  ifstream inputStream(FileName);
  if( !inputStream ) {
    cerr << "Error opening input stream" << endl;
    return 0;
  }
  getline(inputStream,line);
  char *buf=(char *)line.c_str();
  int n=0;
  for(;isspace(*buf);buf++);
  for(;*buf;){
    n++;
    for(;!isspace(*buf)&&*buf;buf++);
    for(;isspace(*buf)&&*buf;buf++);
  }
  inputStream.close();
  return n;
}


DataItem::DataItem()
{
  min=0;
  max=0;
  sum=0;
  maxv=1;
  minv=0;
  nhist=0;
  avg=0;
  scale=true;
}

double DataItem::GetData(unsigned long i)
{
    return data[i];
}

void DataItem::SetData(unsigned long i,double value)
{
    if(value < min) min =value;
    if(value > max) max =value;
    sum +=value;
    sum -=data[i];
    data[i]=value;
    avg=sum / data.size();

}

void DataItem::Add(double value)
{
    if(data.size()==0){
       min =value;
       max =value;
    }
    if(value < min) min =value;
    if(value > max) max =value;
    sum +=value;
    data.push_back(value);
    avg=sum / data.size();
}


double* DataItem::Histogram(int n)
{
//  n minimum 2 olmal�
    if(n==nhist ) return hist;
    for(int i=0;i<n;i++)
      hist[i]=0;
//    double range=(max-min+1)/n;
    double range=(max-min)/n;
    for(int i=0;i<data.size();i++){
      int j=(data[i]-min)/range;
      if(j==n) j=n-1;
      hist[j]+=1.0;
    }
    for(int i=0;i<n;i++){
      hist[i]=hist[i]/data.size();
    }
    nhist=n;
    return hist;
}

void DataItem::Scale( double iminv,double imaxv )
{
    minv=iminv;
    maxv=imaxv;
    Scale();
}

void DataItem::Scale()
{
    if(scale==false) return;
    unsigned long i=0;
    for(i=0;i<data.size(); i++){
      data[i]= (((maxv-minv)*(data[i]-min))/(max-min))+minv;
    }

}

void DataItem::Swap(unsigned long i1,unsigned long i2)
{
    double tmp=data[i1];
    data[i1]=data[i2];
    data[i2]=tmp;
}

double DataItem::GetAvg()
{
    return avg;
}

double DataItem::GetMin()
{
    return min;
}

void DataItem::SetMin(double value)
{
    min=value;
}

double DataItem::GetMax()
{
    return max;
}

void DataItem::SetMax(double value)
{
    max=value;
}

double DataItem::GetMinV()
{
    return minv;
}

void DataItem::SetMinV(double value)
{
    minv=value;
}

double DataItem::GetMaxV()
{
    return maxv;
}

void DataItem::SetMaxV(double value)
{
    maxv=value;
}


DataItem::~DataItem()
{
    data.clear();
}



DataProcess::DataProcess()
{
    //ctor
    srand( time(NULL) );
}


int DataProcess::ReadLine(string line)
{
  double v;
  char *buf=(char *)line.c_str();
  line_data.clear();

  for(;isspace(*buf);buf++);
  for(;*buf;){
    sscanf(buf,"%lf",&v);
    line_data.push_back(v);
    for(;!isspace(*buf)&&*buf;buf++);
    for(;isspace(*buf)&&*buf;buf++);
  }

  if(nItem != line_data.size() )
    return -1;
  for(int i=0;i<nItem;i++)
    items[i]->Add(line_data[i]);
}

// �al���yor
bool DataProcess::LoadRawData(char * filename)
{

  string line;
  char *pos;
  nItem=GetnItem(filename);
  ifstream inputStream(filename);
  if( !inputStream ) {
    cerr << "Error opening input stream" << endl;
    return false;
  }
  for(int i=0;i<nItem;i++)
    items.push_back(new DataItem());

  int i=1;
  while (getline(inputStream,line)){
    if( ReadLine(line)==-1){
       cout << "Error Line : " << i;
       break;
//       return false;
    }
    else
      cout << "Line : " << i << " Processed\n";
    i++;
  };
  nData=i-1;
  inputStream.close();
  return true;

}

void DataProcess::ScaleAll()
{
  for(int i=0; i<nItem; i++){
     items[i]->Scale();
  }
}

bool DataProcess::ParseData()
{
}

DataItem * DataProcess::GetItem(int i)
{
   return items[i];
}

bool DataProcess::WriteScaleParameters(char *FileName)
{
    FILE *out;
    string s;
    string fname=FileName;
    fname+="-scale.txt";
    out=fopen(fname.c_str(),"wt");
    if( out == NULL ) return false;
    fputs("Scaling Parameters Text File generated by FannTool \n",out);
    fputs("Column No  ( Minimum , Maximum ) --> ( Minimum Out , Maximum Out ) \n",out);
    for(int j=0; j<nItem; j++){
      if(items[j]->scale)
         fprintf(out,"%d ( %f , %f ) --> ( %f , %f ) \n",j+1 ,items[j]->GetMin(),items[j]->GetMax(),items[j]->GetMinV(),items[j]->GetMaxV());
       else
         fprintf(out,"%d ( %f , %f ) --> Not Scaled \n",j+1 ,items[j]->GetMin(),items[j]->GetMax());

    }


    fclose(out);
    return true;
}

bool DataProcess::WriteData(char *FileName,int nOut, float pTrain)
{
    FILE *out;
    string s;
    unsigned long nTrainData,nTestData;
    // pTrain = 0.7 ise 0.6 ya yuvarl�yor gibi
    if(pTrain > 1.0) pTrain=1.0;
    if(pTrain < 0.5) pTrain=0.5;
    nTrainData=(unsigned long)(GetNData()*pTrain);
    nTestData=(unsigned long)(GetNData()-nTrainData);
    int nInput=nItem-nOut;
    char buf[50];
    string fname=FileName;
    fname+="-train.dat";
    out=fopen(fname.c_str(),"w");
    if( out == NULL ) return false;
    fprintf(out,"%d %d %d \n",nTrainData,nInput,nOut);

    for(unsigned long n=0; n<nTrainData; n++){
       s="";
      for(int i=0; i<nInput; i++){
        sprintf(buf,"%f ",items[i]->GetData(n));
        s+=buf;
      }
      s+="\n";
      fputs(s.c_str(),out);
      s="";
      for(int i=nInput; i<nItem; i++){
        sprintf(buf,"%f ",items[i]->GetData(n));
        s+=buf;
      }
      s+="\n";
      fputs(s.c_str(),out);


    }
    fclose(out);

    if(nTestData){
      string fname=FileName;
      fname+="-test.dat";
      out=fopen(fname.c_str(),"w");
      if( out == NULL ) return false;
      fprintf(out,"%d %d %d \n",nTestData,nInput,nOut);
      for(unsigned long n=nTrainData; n<nData; n++){
         s="";
         for(int i=0; i<nInput; i++){
           sprintf(buf,"%f ",items[i]->GetData(n));
           s+=buf;
         }
         s+="\n";
         fputs(s.c_str(),out);
         s="";
         for(int i=nInput; i<nItem; i++){
          sprintf(buf,"%f ",items[i]->GetData(n));
          s+=buf;
        }
        s+="\n";
        fputs(s.c_str(),out);
      }
      fclose(out);

    }
    return true;


}

void DataProcess::Shuffle()
{
	unsigned long i= 0, elem, swap;
	for(unsigned long i= 0; i < nData /2 ; i++){
      swap = (unsigned long) (rand() % (unsigned long)nData);
      if(i!=swap){
        for(int j=0; j<nItem; j++)
          items[j]->Swap(i,swap);
      }



    }

}


DataProcess::~DataProcess()
{
   for(int j=0; j<nItem; j++)
      delete (items[j]);
}

//  Time Series Reader

TimeSeri::TimeSeri()
{
  min=0;
  max=0;
  maxv=1;
  minv=0;
  scale=true;
  shuffle=false;
  count=
  nInput=1;
  nOutput=1;
}

bool TimeSeri::LoadRawData(char *file)
{
   FILE *fp;
   fp=fopen(file,"r");
   if(!fp) return false;
   data.clear();
   double tmp;
   max=0.0;
   min=0.0;
   count=0;
   char Buf[30];
   do{
     if(fgets(Buf,30,fp)){
       tmp=atof(Buf);
       if(count==0)
         min=tmp;
       if(min>tmp)
         min=tmp;
       if(max<tmp)
         max=tmp;
       data.push_back(tmp);
       count++;
     }
   }while(!feof(fp));
   fclose(fp);
   if(count)
     return true;
   else
     return false;

}

/*
void TimeSeri::Normalize()
{
// veriler -0.8 ile 0.8 ars�na �ekiliyor
 double range=(max-min)/1.6;
 for(long i=0;i<count;i++)
   data[i]=((data[i]-min)/range)-0.8;
}

void TimeSeri::Scale( double iminv,double imaxv )
{
    minv=iminv;
    maxv=imaxv;
    Scale();
}

void TimeSeri::Scale()
{
    if(scale==false) return;
    unsigned long i=0;
    for(i=0;i<data.size(); i++){
      data[i]= (((maxv-minv)*(data[i]-min))/(max-min))+minv;
    }

}

*/

bool TimeSeri::WriteScaleParameters(char *FileName)
{
    FILE *out;
    string s;
    string fname=FileName;
    fname+="-scale.txt";
    out=fopen(fname.c_str(),"wt");
    if( out == NULL ) return false;
    fputs("Scaling Parameters Text File generated by FannTool 1.0 \n",out);
    fputs("Column No  ( Minimum , Maximum ) --> ( Minimum Out , Maximum Out ) \n",out);
    for(int j=0; j<nInput+nOutput; j++){
      if(scale)
         fprintf(out,"%d ( %f , %f ) --> ( %f , %f ) \n",j+1 ,GetMin(),GetMax(),GetMinV(),GetMaxV());
       else
         fprintf(out,"%d ( %f , %f ) --> Not Scaled \n",j+1 ,GetMin(),GetMax());

    }


    fclose(out);
    return true;
}




bool TimeSeri::WriteData(char *file, float pTrain)
{
   // FILE *out;
   char buf[100];
   string tmp;
   string s;
   vector<string> indata;
   vector<string> outdata;


   long ndata=count-nInput-nOutput+1;

   for(long i=0;i<ndata;i++){
     tmp="";
     for(int j=0;j<nInput;j++){
       sprintf(buf,"%lf ",data[i+j] );
       tmp+=buf;
     }
     tmp+="\n";
     indata.push_back(tmp);
     tmp="";
     for(int j=nInput;j<nInput+nOutput;j++){
       sprintf(buf,"%lf ",data[i+j] );
       tmp+=buf;
     }
     tmp+="\n";
     outdata.push_back(tmp);
   }
// Shuffle K�sm�
    if(shuffle){
      unsigned long elem, swap;
	  for(unsigned long i= 0; i < ndata /2 ; i++){
        swap = (unsigned long) (rand() % (unsigned long)ndata);
        if(i!=swap){
          tmp=indata[i];
          indata[i]=indata[swap];
          indata[swap]=tmp;
          tmp=outdata[i];
          outdata[i]=outdata[swap];
          outdata[swap]=tmp;

        }
	  }

    }



//
    FILE *out;

    unsigned long nTrainData,nTestData;
    // pTrain = 0.7 ise 0.6 ya yuvarl�yor gibi
    if(pTrain > 1.0) pTrain=1.0;
    if(pTrain < 0.5) pTrain=0.5;
    nTrainData= (unsigned long)(pTrain*ndata);
    nTestData=(unsigned long)(ndata-nTrainData);
    string fname=file;
    fname+="-train.dat";
    out=fopen(fname.c_str(),"w");
    if( out == NULL ) return false;
    fprintf(out,"%d %d %d \n",nTrainData,nInput,nOutput);

    for(unsigned long n=0; n<nTrainData; n++){
       s=indata[n];
      fputs(s.c_str(),out);
      s=outdata[n];
      fputs(s.c_str(),out);
    }
    fclose(out);

    if(nTestData){
      string fname=file;
      fname+="-test.dat";
      out=fopen(fname.c_str(),"w");
      if( out == NULL ) return false;
      fprintf(out,"%d %d %d \n",nTestData,nInput,nOutput);
      for(unsigned long n=nTrainData; n<ndata; n++){
        s=indata[n];
        fputs(s.c_str(),out);
        s=outdata[n];
        fputs(s.c_str(),out);
      }
      fclose(out);

    }
//
   return true;


}




TimeSeri::~TimeSeri()
{
  data.clear();

}


