#include "include/sample.hpp"

/**
* @brief Definition of the `Sample` class, and functions for saving and loading samples
*/

using namespace std;


Sample::Sample(string filename, string reference, unordered_set<int> mask){
    char ch;
    fstream fin(filename, fstream::in);
    //Deal with the header first
    //Assume last pipe separated value in header is UUID (at least for now)
    while(fin >> noskipws >> ch){
        if(ch == '\n'){
            //Line has ended
            break;
        }
        if(ch == '|'){
            //New pipe, so reset
            uuid = "";
        }
        else{
            //Add to the str
            uuid += ch;
        }
    }

    int i = 0;
    while (fin >> noskipws >> ch) {
        if(ch == '\n' || ch == '\r'){ 
            //We don't care about these endl chars
            continue;
        }
        if(mask.contains(i)){
            //Skip if the exclude mask is at this point
            i++;
            continue;
        }
        if(ch != reference[i]){
            //We have a difference from reference, so add to appropriate set
            switch (ch) {
                case 'A':
                    A.insert(i);
                    break;
                case 'C':
                    C.insert(i);
                    break;
                case 'G':
                    G.insert(i);
                    break;
                case 'T':
                    T.insert(i);
                    break;
                case 'N':
                    N.insert(i);
                    break;
                case '-':
                    N.insert(i);
                    break;
                case 'X':
                    N.insert(i);
                    break;
                case 'O':
                    N.insert(i);
                    break;
                case 'Z':
                    N.insert(i);
                    break;
            }
        }
        i++;
    }
    fin.close();
}

Sample::Sample(unordered_set<int> a, unordered_set<int> c, unordered_set<int> g, unordered_set<int> t, unordered_set<int> n){
    A = a;
    C = c;
    G = g;
    T = t;
    N = n;
}

bool Sample::operator== (const Sample &s2) const{
    bool check =  (uuid == s2.uuid);
    check = check && (A == s2.A);
    check = check && (C == s2.C);
    check = check && (G == s2.G);
    check = check && (T == s2.T);
    check = check && (N == s2.N);
    return check;
}

int Sample::dist(Sample* sample, int cutoff){
    unordered_set<int> acc;
    acc = dist_x(A, N, sample->A, sample->N, acc, cutoff);
    acc = dist_x(C, N, sample->C, sample->N, acc, cutoff);
    acc = dist_x(T, N, sample->T, sample->N, acc, cutoff);
    acc = dist_x(G, N, sample->G, sample->N, acc, cutoff);
    return acc.size();
}

unordered_set<int> Sample::dist_x(unordered_set<int> this_x, unordered_set<int> this_n, unordered_set<int> sample_x, unordered_set<int> sample_n, unordered_set<int> acc, int cutoff){
    //Increment cutoff so we can reject distances > cutoff entirely
    cutoff++;
    for(const int& elem: this_x){
        if(acc.size() == cutoff){
            return acc;
        }
        if(!sample_x.contains(elem)){ 
            //Not a in sample
            if(!sample_n.contains(elem)){
                //Not an N comparison either, so add
                acc.insert(elem);
            }
        }
    }
    for(const int& elem: sample_x){
        if(acc.size() == cutoff){
            return acc;
        }
        if(!this_x.contains(elem)){
            //Not a in sample
            if(!this_n.contains(elem)){
                //Not an N comparison either, so add
                acc.insert(elem);
            }
        }
    }

    return acc;
}

void save_n(unordered_set<int> to_save, string filename){
    fstream out(filename, fstream::binary | fstream::out);

    for(const int elem: to_save){
        const int *p = &elem;
        const char *p_ = (char *) p;
        out.write(p_, 4);
    }

    out.close();
}

unordered_set<int> load_n(string filename){
    //ate flag seeks to the end of the file
    fstream in(filename, fstream::binary | fstream::in | fstream::ate);
    //Get the size of the file in bytes (we have to convert this as 1 int == 4 bytes)
    int size = in.tellg();
    in.seekg(0); //Go back to the start so we can read

    unordered_set<int> output;

    for(int j=0;j<size/4;j++){
        //Read 4 characters from the file into a char array
        char i[4];
        in.read(i, 4);

        //Convert the char array into an int and insert
        int *p;
        p = (int *) i;
        output.insert(*p);
    }

    in.close();

    return output;
}

void save(string filename, Sample* sample){
    //Append the sample UUID to the filename to save as such
    if(filename[filename.size()-1] != '/'){
        //No trailing / so add
        filename += '/';
    }
    filename += sample->uuid;
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    for(int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        //Find out which one we need to save
        unordered_set<int> toSave;
        switch (types.at(i)) {
            case 'A':
                toSave = sample->A;
                break;
            case 'C':
                toSave = sample->C;
                break;
            case 'G':
                toSave = sample->G;
                break;
            case 'T':
                toSave = sample->T;
                break;
            case 'N':
                toSave = sample->N;
                break;
        }
        save_n(toSave, f);
    }
}

Sample* readSample(string filename){
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    vector<unordered_set<int>> loading;
    for(int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        loading.push_back(load_n(f));
    }
    Sample *s = new Sample(loading.at(0), loading.at(1), loading.at(2), loading.at(3), loading.at(4));
    
    //Get the UUID from the filename
    string uuid;
    if(filename.at(filename.size()-1) == '/'){
        //Remove trailing / if req
        filename.pop_back();
    }
    for(int i=0;i<filename.size();i++){
        if(filename.at(i) == '/'){
            //We only want the last part, so reset
            uuid = "";
        }
        else{
            uuid += filename.at(i);
        }
    }
    s->uuid = uuid;
    return s;
}

string load_reference(string filename){
    //Load reference
    string reference = "";
    char ch;
    fstream fin(filename, fstream::in);

    //First line is the header, but for this we don't care about it
    while(fin >> noskipws >> ch){
        if(ch == '\n' || ch == '\r'){
            break;
        }
    }
    //Now just parse the file
    while (fin >> noskipws >> ch) {
        if(ch == '\n' || ch =='\r'){
            //We don't want these
            continue;
        }
        reference += ch;
    }
    
    fin.close();

    return reference;
}

unordered_set<int> load_mask(string filename){
    //Load the exclude mask
    unordered_set<int> mask;
    if(filename == "ignore"){
        //If the user has requested to ignore this, then do.
        return mask;
    }
    fstream fin2(filename, fstream::in);
    string acc;
    char ch;
    while (fin2 >> noskipws >> ch) {
        if(ch == '\n'){
            mask.insert(stoi(acc));
            acc = "";
        }
        else{
            acc += ch;
        }
    }
    fin2.close();

    return mask;
}
