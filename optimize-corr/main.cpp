/*****
Etienne JACOB, 13/08/2016
*****
Off files and correspondences given.
Trying to find out bad correspondences.
And trying to correct them.
---
C++11 features are used
*****/

#include <bits/stdc++.h>
using namespace std;

const string path = "C:/Users/Etienne/Desktop/before_mincut/meshes/";
const string path2 = "C:/Users/Etienne/Desktop/before_mincut/data/";

const int MAX_SIZE_V = 7000;
const int MAX_SIZE_T = 15000;
const int SAMPLE_SIZE = 500;
const float INFINITE = 1e5;
const float VOTE_LIM = 0.2;

float vert[MAX_SIZE_V][3][2];
int triangles[MAX_SIZE_T][3][2];
int n[2],p[2],m[2];
vector<int> graphMesh[MAX_SIZE_V][2];
float geoDistances[SAMPLE_SIZE][MAX_SIZE_V][2];

vector<int> samplingVec;
map<int,int> indexS;
string header;
int sampleEval[SAMPLE_SIZE];


float shortDist(const int& i1,const int& i2,const int& mesh){
    return sqrt((vert[i1][0][mesh]-vert[i2][0][mesh])*(vert[i1][0][mesh]-vert[i2][0][mesh]) + (vert[i1][1][mesh]-vert[i2][1][mesh])*(vert[i1][1][mesh]-vert[i2][1][mesh]) + (vert[i1][2][mesh]-vert[i2][2][mesh])*(vert[i1][2][mesh]-vert[i2][2][mesh]));
}

bool isNew(const int& v,const int& mesh, const int& val){
    for(int i=0;i<int(graphMesh[v][mesh].size());i++){
        if(graphMesh[v][mesh][i]==val){
            return false;
        }
    }
    return true;
}

void loadMesh(const string& name,const int& mesh){
    string filepath = path + name + ".off";
    ifstream in (filepath);

    in>>header;

    in>>n[mesh]>>p[mesh]>>m[mesh];
    for(int i=0;i<n[mesh];i++){
        for(int j=0;j<3;j++){
            in>>vert[i][j][mesh];
        }
    }
    for(int i=0;i<p[mesh];i++){
        int waste;in>>waste;
        int v1,v2,v3;in>>v1>>v2>>v3;
        if(isNew(v1,mesh,v2)) graphMesh[v1][mesh].push_back(v2);
        if(isNew(v2,mesh,v1)) graphMesh[v2][mesh].push_back(v1);
        if(isNew(v3,mesh,v2)) graphMesh[v3][mesh].push_back(v2);
        if(isNew(v2,mesh,v3)) graphMesh[v2][mesh].push_back(v3);
        if(isNew(v1,mesh,v3)) graphMesh[v1][mesh].push_back(v3);
        if(isNew(v3,mesh,v1)) graphMesh[v3][mesh].push_back(v1);
        triangles[i][0][mesh]=v1;
        triangles[i][1][mesh]=v2;
        triangles[i][2][mesh]=v3;
    }

    if(int(samplingVec.size())<=0){
        vector<int> listIndex;
        for(int i=0;i<n[0];i++){
            listIndex.push_back(i);
        }

        for(int i=n[0]-1;i>=0;i--){
            int k = rand()%(i+1);
            int aux = listIndex[i];
            listIndex[i]=listIndex[k];
            listIndex[k]=aux;
        }

        for(int i=0;i<min(int(listIndex.size()),SAMPLE_SIZE);i++){
            samplingVec.push_back(listIndex[i]);
            indexS[listIndex[i]]=i;
        }
        sort(samplingVec.begin(),samplingVec.end());
    }
}

vector<int> loadCorr(const string& name){
    string filepath = path2 + name + ".txt";
    ifstream in (filepath);

    int sz;
    in>>sz;

    vector<int> corr;

    for(int i=0;i<sz;i++){
        int j;in>>j;
        corr.push_back(j-1);
    }

    return corr;
}

void saveCorr(const string& name,const vector<int>& corr){
    string filepath = path2 + name + ".txt";
    ofstream out (filepath);

    int sz=corr.size();
    out<<sz<<"\n";

    for(int i=0;i<sz;i++){
        out<<corr[i]+1<<" ";
    }
}

set<int> bfs(const int& start,const int& nb,const int& mesh){
    set<int> res;
    vector<bool> visited(n[mesh],0);

    queue<int> frontier;
    frontier.push(start);
    visited[start]=true;

    while(!frontier.empty() && int(res.size())<nb){
        int next = frontier.front();
        for(int i = 0;i < int(graphMesh[next][mesh].size());i++){
            int k = graphMesh[next][mesh][i];
            if(!visited[k]){
                frontier.push(k);
                visited[k]=true;
            }
        }
        res.insert(next);
        frontier.pop();
    }
    return res;
}

long long evaluate(const int& p,const vector<int>& corr){
    long long res = 250;
    float dThreshold = VOTE_LIM;
    for(int i=0;i<SAMPLE_SIZE;i++){
        if(abs(geoDistances[i][p][0] - geoDistances[i][corr[p]][1])<dThreshold){
            res--;
        }
    }
    return res;
}

void reconstruct(const set<int>& points,const string& name,const int& mesh){
    string filepath = path + name + "_bfs.off";
    ofstream out (filepath);

    set<int>::iterator it;
    map<int,int> newindex;
    int cnt = 0;
    for(it = points.begin();it != points.end();it++){
        newindex[*it]=cnt;
        cnt++;
    }
    //sort(pointsVec.begin(),pointsVec.end());

    out<<header<<"\n";

    int nn = int(points.size());
    int pp = 0;

    vector<bool> acceptedTriangle(p[mesh],0);
    for(int i=0;i<p[mesh];i++){
        if((points.count(triangles[i][0][mesh])>0)&&(points.count(triangles[i][1][mesh])>0)&&(points.count(triangles[i][2][mesh])>0)){
            acceptedTriangle[i]=true;
            pp++;
        }
    }

    out<<nn<<" "<<pp<<" "<<0<<"\n";
    for(int i=0;i<n[mesh];i++){
        if(points.count(i)>0){
            out<<vert[i][0][mesh]<<" "<<vert[i][1][mesh]<<" "<<vert[i][2][mesh]<<"\n";
        }
    }
    for(int i=0;i<p[mesh];i++){
        if(acceptedTriangle[i]){
            out<<3<<" "<<newindex[triangles[i][0][mesh]]<<" "<<newindex[triangles[i][1][mesh]]<<" "<<newindex[triangles[i][2][mesh]]<<"\n";
        }
    }
}

int reconstructCorrectGeo(const string& name,const int& threshold,vector<int>& corr,const& opt){
    string filepath = path + name + "_cgeo.off";
    ofstream out (filepath);

    vector<bool> state(n[0],0);

    int cnt = 0;
    map<int,int> newindex;
    for(int i=0;i<n[0];i++){
        int before = evaluate(i,corr);
        if(before<threshold){
            state[i]=true;
            newindex[i]=cnt;
            cnt++;
        }
        else{
            int theMin = before;
            int jmin = corr[i];
            if(opt){
                for(int j=0;j<n[1];j++){
                    corr[i]=j;
                    int aux = evaluate(i,corr);
                    if(aux<theMin){
                        theMin = aux;
                        jmin=j;
                    }
                }
            }
            corr[i]=jmin;
            state[i]=(theMin<threshold);
            cnt+=state[i];
            if(i%25==0 && opt){cout << before - theMin <<" "<<-before<<" "<<-theMin<<"\n";}
        }
    }
    //sort(pointsVec.begin(),pointsVec.end());

    out<<header<<"\n";

    int nn = cnt;
    int pp = 0;

    vector<bool> acceptedTriangle(p[0],0);
    for(int i=0;i<p[0];i++){
        if((state[triangles[i][0][0]])&&(state[triangles[i][1][0]])&&(state[triangles[i][2][0]])){
            acceptedTriangle[i]=true;
            pp++;
        }
    }

    out<<n[0]<<" "<<p[0]<<" "<<0<<"\n";
    for(int i=0;i<n[0];i++){
        if(true){
            out<<vert[i][0][0]<<" "<<vert[i][1][0]<<" "<<vert[i][2][0]<<"\n";
        }
    }
    for(int i=0;i<p[0];i++){
        if(acceptedTriangle[i]){
            out<<3<<" "<<triangles[i][0][0]<<" "<<triangles[i][1][0]<<" "<<triangles[i][2][0]<<" "<<"0.0"<<" "<<"0.0"<<" "<<"1.0"<<" "<<"1.0"<<"\n";
        }
        else{
            out<<3<<" "<<triangles[i][0][0]<<" "<<triangles[i][1][0]<<" "<<triangles[i][2][0]<<" "<<"1.0"<<" "<<"0.0"<<" "<<"0.0"<<" "<<"1.0"<<"\n";
        }
    }
    return nn;
}

void dijkstraFill(const int& start,const int& posTab,const int& mesh){
    for(int i=0;i<n[mesh];i++){
        geoDistances[posTab][i][mesh] = INFINITE;
    }
    geoDistances[posTab][start][mesh] = 0;

    vector<bool> alreadyComputed(n[mesh],0);

    priority_queue<pair<float,int> > frontier;
    frontier.push(make_pair(0,start));

    while(!frontier.empty()){
        float nextVal = -frontier.top().first;
        int nextInd = frontier.top().second;
        frontier.pop();
        if(!alreadyComputed[nextInd]){
            for(int i = 0;i < int(graphMesh[nextInd][mesh].size());i++){
                int k = graphMesh[nextInd][mesh][i];
                float newDist =  nextVal + shortDist(nextInd,k,mesh);
                if(newDist<geoDistances[posTab][k][mesh]){
                    geoDistances[posTab][k][mesh] = newDist;
                    frontier.push(make_pair(-newDist,k));
                }
            }
        }
        alreadyComputed[nextInd]=true;
    }
}

float simpleGlobalEval(const vector<int>& corr){
    double res = 0;
    for(int i=0;i<n[0];i++){
        res += shortDist(i,corr[i],1)/n[0];
    }
    return res;
}

int defineTs(const vector<int>& corr){
    vector<int> aux;
    int SZ = 50;
    for(int i=0;i<SZ;i++){
        aux.push_back(evaluate(rand()%(n[0]),corr));
    }
    sort(aux.begin(),aux.end());
    return aux[7*SZ/8];
}

void refineSamples(const int& nSteps,const vector<int>& corr){
    for(int i=0;i<SAMPLE_SIZE;i++){
        sampleEval[i]=evaluate(samplingVec[i],corr);
    }
    for(int k=0;k<nSteps;k++){
        int theMax = -1e5;
        int iMax = 0;
        for(int i=0;i<SAMPLE_SIZE;i++){
            if(sampleEval[i]>theMax){
                iMax = i;
                theMax = sampleEval[i];
            }
        }
        /*
        int theMin = 1e5;
        int jmin = corr[iMax];
        for(int j=0;j<n[1];j++){
            corr[iMax]=j;
            int aux = evaluate(iMax,corr);
            if(aux<theMin){
                theMin = aux;
                jmin=j;
            }
        }
        corr[iMax]=jmin;*/

        samplingVec[iMax] = rand()%(n[0]);

        for(int i=0;i<SAMPLE_SIZE;i++){
            if(i==iMax){
                ;
            }
            else{
                if(abs(geoDistances[i][samplingVec[iMax]][0] - geoDistances[i][corr[samplingVec[iMax]]][1])<VOTE_LIM){
                    sampleEval[i]++;
                }
            }
        }

        dijkstraFill(samplingVec[iMax],iMax,0);
        dijkstraFill(corr[samplingVec[iMax]],iMax,0);

        for(int i=0;i<SAMPLE_SIZE;i++){
            if(i==iMax){
                sampleEval[i]=evaluate(samplingVec[i],corr);
            }
            else{
                if(abs(geoDistances[i][samplingVec[iMax]][0] - geoDistances[i][corr[samplingVec[iMax]]][1])<VOTE_LIM){
                    sampleEval[i]--;
                }
            }
        }

    }
}

void refineSamples2(const int& nSteps,vector<int>& corr){
    for(int i=0;i<SAMPLE_SIZE;i++){
        sampleEval[i]=evaluate(samplingVec[i],corr);
    }
    for(int k=0;k<nSteps;k++){
        int theMax = -1e5;
        int iMax = 0;
        for(int i=0;i<SAMPLE_SIZE;i++){
            if(sampleEval[i]>theMax){
                iMax = i;
                theMax = sampleEval[i];
            }
        }

        int theMin = 1e5;
        int jmin = corr[iMax];
        for(int j=0;j<n[1];j++){
            corr[iMax]=j;
            int aux = evaluate(iMax,corr);
            if(aux<theMin){
                theMin = aux;
                jmin=j;
            }
        }
        corr[iMax]=jmin;

        //samplingVec[iMax] = rand()%(n[0]);

        for(int i=0;i<SAMPLE_SIZE;i++){
            if(i==iMax){
                ;
            }
            else{
                if(abs(geoDistances[i][samplingVec[iMax]][0] - geoDistances[i][corr[samplingVec[iMax]]][1])<VOTE_LIM){
                    sampleEval[i]++;
                }
            }
        }

        //dijkstraFill(samplingVec[iMax],iMax,0);
        dijkstraFill(corr[samplingVec[iMax]],iMax,0);

        for(int i=0;i<SAMPLE_SIZE;i++){
            if(i==iMax){
                sampleEval[i]=evaluate(samplingVec[i],corr);
            }
            else{
                if(abs(geoDistances[i][samplingVec[iMax]][0] - geoDistances[i][corr[samplingVec[iMax]]][1])<VOTE_LIM){
                    sampleEval[i]--;
                }
            }
        }

    }
}

int main(){
    srand(time(0));

    vector<int> corr = loadCorr("corr41_1");
    loadMesh("gtfaustoff_all41",0);
    loadMesh("gtfaustoff_all1",1);
    cout << "loading ok ... \n\n";

    for(int i=0;i<SAMPLE_SIZE;i++){
        dijkstraFill(samplingVec[i],i,0);
        cout << "Dijkstra from " << i << "/" << 499 << "done.\n";
    }

    for(int i=0;i<SAMPLE_SIZE;i++){
        dijkstraFill(corr[samplingVec[i]],i,1);
        cout << "Dijkstra from " << i << "/" << 499 << "done.\n";
    }

    cout << "Voters refinement ...\n";
    refineSamples(200,corr);
    //refineSamples2(200,corr);
    cout << "Voters refinement done.\n";

    cout << endl << simpleGlobalEval(corr) << endl << endl;

    int ts = defineTs(corr);

    int nbCorrect = reconstructCorrectGeo("gtfaustoff_all41_2",ts,corr,1);
    nbCorrect = reconstructCorrectGeo("gtfaustoff_all41_2",ts-50,corr,0);
    cout << "Correct : " << nbCorrect << " / " << n[0] << endl;

    cout << endl << simpleGlobalEval(corr) << endl << endl;

    saveCorr("corr41_1__2",corr);

    return 0;
}
