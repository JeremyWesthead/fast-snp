#include <ios>
#include <ostream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <vector>
#include <unordered_set>
#include <string>
#include <string.h>
#include <chrono>
#include <filesystem>
#include <thread>
#include <tuple>
#include <mutex>

namespace fs = std::filesystem;
using namespace std;

mutex mutex_lock;

class Sample{
    public:
        unordered_set<int> A;
        unordered_set<int> C;
        unordered_set<int> G;
        unordered_set<int> T;
        unordered_set<int> N;
        string uuid;

        Sample(string filename, string reference, unordered_set<int> mask){
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
                if(ch == '\n'){ 
                    //We don't care about these endl chars
                    continue;
                }
                if(mask.contains(i)){
                    //Skip if the exclude mask is at this point
                    i++;
                    continue;
                }
                if(ch != reference[i]){
                    // cout << ch << "| |" << reference[i] << " " << (ch == reference[i]) << endl;
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
                    }
                }
                i++;
            }
            fin.close();
        }

        Sample(unordered_set<int> a, unordered_set<int> c, unordered_set<int> g, unordered_set<int> t, unordered_set<int> n){
            A = a;
            C = c;
            G = g;
            T = t;
            N = n;
        }

        int dist(Sample* sample, int cutoff){
            unordered_set<int> acc;
            acc = dist_x(A, N, sample->A, sample->N, acc, cutoff);
            acc = dist_x(C, N, sample->C, sample->N, acc, cutoff);
            acc = dist_x(T, N, sample->T, sample->N, acc, cutoff);
            acc = dist_x(G, N, sample->G, sample->N, acc, cutoff);
            return acc.size();
        }

        unordered_set<int> dist_x(unordered_set<int> this_x, unordered_set<int> this_n, unordered_set<int> sample_x, unordered_set<int> sample_n, unordered_set<int> acc, int cutoff){
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
}};

void save(string filename, Sample* sample){
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
        ofstream out(f, ios::binary);
        for(const int elem: toSave){
            out << elem << endl;
        }
        out << EOF;
        out.flush();
        out.close();
    }
}

Sample* readSample(string filename){
    vector<char> types = {'A', 'C', 'G', 'T', 'N'};
    vector<unordered_set<int>> loading;
    for(int i=0; i<types.size(); i++){
        string f = filename + '.' + types.at(i);
        string acc;
        char ch;
        fstream fin(f, fstream::in | fstream::binary);
        unordered_set<int> toLoad;
        int counter = 0;
        while (fin >> noskipws >> ch) {
            if(ch == '\n'){
                //This is an endl, so store whats currently in acc as an int and reset
                toLoad.insert(stoi(acc));
                acc = "";
            }
            else{
                //Just add to acc
                acc += ch;
            }
        }
        fin.close();

        loading.push_back(toLoad);
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

uint64_t sysTime() {
  using namespace std::chrono;
  return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

bool check_bulk_load(int nargs, const char* args[]){
    //Check if this is for bulk loading from FASTA --> save
    string flag =  "--bulk_load";
    for(int i=0;i<nargs;i++){
        if(args[i] == flag){
            return true;
        }
    }
    return false;
}

bool check_add_sample(int nargs, const char* args[]){
    //Check if this is for comparing against existing saves (and saving)
    string flag = "--add";
    for(int i=0;i<nargs;i++){
        if(args[i] == flag){
            return true;
        }
    }
    return false;
}

bool check_compute(int nargs, const char* args[]){
    //Checking if this is just asking to recompute all saved samples with a cutoff
    string flag = "--compute";
    for(int i=0;i<nargs;i++){
        if(args[i] == flag){
            return true;
        }
    }
    return false;
}

void load_n(vector<string> paths, string reference, unordered_set<int> mask){
    //Load all of the samples in `paths`
    for(const string path: paths){
        Sample *s = new Sample(path, reference, mask);
        
        mutex_lock.lock();
            save("saves/"+s->uuid, s);
        mutex_lock.unlock();
    }
}

void bulk_load(string path, string reference, unordered_set<int> mask, int thread_count){
    //Take a path to a file specifying which files to load
    //Parse and save, no comparisons
    //Read the file given as an arg, treating each line as a new filepath
    vector<string> filepaths;
    char ch;
    fstream pathsFile(path, fstream::in);
    string f;
    while(pathsFile >> noskipws >> ch){
        if(ch == '\n'){
            //Only add if the path found is not empty
            if(f.find_first_not_of(' ') != string::npos){
                //Also check for .gitkeep
                if(f != ".gitkeep"){
                    filepaths.push_back(f);
                }
            }
            f = "";
        }
        else{
            f += ch;
        }
    }
    //Add the last one
    if(f != ""){
        filepaths.push_back(f);
    }
    pathsFile.close();

    //Do comparisons with multithreading
    int chunk_size = filepaths.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<string> these(filepaths.begin() + i*chunk_size, filepaths.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(load_n, these, reference, mask));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)
    for(int i=chunk_size*thread_count;i<filepaths.size();i++){
        Sample *s = new Sample(filepaths.at(i), reference, mask);
        mutex_lock.lock();
            save("saves/"+s->uuid, s);
        mutex_lock.unlock();
    }

    //Join the threads
    for(int i=0;i<threads.size();i++){
        threads.at(i).join();
    }
}

void add_sample(string path, string reference, unordered_set<int> mask){
    //Parse a new sample
    //Compare it to every saved sample, then save it too
    Sample *s = new Sample(path, reference, mask);

    //Find all saves
    unordered_set<string> saves;
    for (const auto & entry : fs::directory_iterator("./saves")){
        string p = entry.path();    
        //Check for .gitkeep or nothing (we want to ignore this)
        if(p == "./saves/.gitkeep" || p == ".saves/"){
            continue;
        }
        
        //Remove `.A` etc
        p.pop_back();
        p.pop_back();

        //Only add if the path found is not empty
        if(p.find_first_not_of(' ') != string::npos){
            saves.insert(p);
        }
    }
    //Load them and perform comparisons
    for(const string elem: saves){
        Sample *save = readSample(elem);
        cout << save->uuid << " " << s->dist(save, 2000000) << endl;
    }

    //Save the new sample
    save("saves/"+s->uuid, s);
}

void save_comparisons(vector<tuple<string, string, int>> comparisons){
    //Save some comparisons to disk in a threadsafe manner
    mutex_lock.lock();
        fstream output("outputs/all.txt", fstream::app);
        for(const tuple<string, string, int> elem: comparisons){
            output << get<0>(elem) << " " << get<1>(elem) << " " << get<2>(elem) << endl;
        }
        output.close();
    mutex_lock.unlock();
}

void do_comparisons(vector<tuple<Sample*, Sample*>> comparisons, int cutoff){
    //To be used by Thread to do comparisons in parallel
    vector<tuple<string, string, int>> distances;
    for(int i=0;i<comparisons.size();i++){
        Sample *s1 = get<0>(comparisons.at(i));
        Sample *s2 = get<1>(comparisons.at(i));
        int dist = s1->dist(s2, cutoff);
        if(dist > cutoff){
            //Further than cutoff so ignore
            continue;
        }
        distances.push_back(make_tuple(s1->uuid, s2->uuid, dist));

        if(distances.size() == 1000){
            //We have a fair few comparisons now, so save
            save_comparisons(distances);
            //Clear for continuing
            distances = {};

        }
        //Adding to file separately might be slightly faster (due to mutex), but this involves constructing
        //In-RAM comparisons which scales poorly as RAM is limiting factor
        // mutex_lock.lock();
        //     fstream output("outputs/all.txt", fstream::app);
        //     output << s1->uuid << " " << s2->uuid << " " << dist << endl;
        //     output.close();
        // mutex_lock.unlock();
    }
    //And save the last few (if existing)
    save_comparisons(distances);
}

void compute(int cutoff){
    //Compute pairwise matrix of all saved genomes with a given cutoff
    //Find all saves
    unordered_set<string> saves;
    for (const auto & entry : fs::directory_iterator("./saves")){
        string p = entry.path();    
        //Check for .gitkeep or nothing (we want to ignore this)
        if(p == "./saves/.gitkeep" || p == ".saves/"){
            continue;
        }
        
        //Remove `.A` etc
        p.pop_back();
        p.pop_back();

        //Only add if the path found is not empty
        if(p.find_first_not_of(' ') != string::npos){
            saves.insert(p);
        }
    }

    vector<Sample*> samples;
    for(const string elem: saves){
        Sample *s = readSample(elem);
        samples.push_back(s);
    }
}

void compute_loaded(int cutoff, vector<Sample*> samples, int thread_count){
    //Version of compute() without reading from disk
    //Utilise multithreading for speed

    //Construct list of comparisons (but don't actually do any of them yet)
    vector<tuple<Sample*, Sample*>> comparisons;
    unordered_set<string> seen;
    for(int i=0;i<samples.size();i++){
        Sample *s1 = samples.at(i);
        seen.insert(s1->uuid);
        for(int j=0;j<samples.size();j++){
            Sample *s2 = samples.at(j);
            if(seen.contains(s2->uuid)){
                //We've already done this comparison
                continue;
            }
            else{
                comparisons.push_back(make_tuple(s1, s2));
            }
        }
    }
    cout << "Comparing " << samples.size() << " for a total of " << comparisons.size() << " comparisons" << endl;

    //Clear output file ready for thread-by-thread appending
    fstream output("outputs/all.txt", fstream::out);
    output.close();

    //Do comparisons with multithreading
    int chunk_size = comparisons.size() / thread_count;
    vector<thread> threads;
    for(int i=0;i<thread_count;i++){
        vector<tuple<Sample*, Sample*>> these(comparisons.begin() + i*chunk_size, comparisons.begin() + i*chunk_size + chunk_size);
        threads.push_back(thread(do_comparisons, these, cutoff));
    }
    //Catch ones missed at the end due to rounding (doing on main thread)

    for(int i=chunk_size*thread_count;i<comparisons.size();i++){
        tuple<Sample*, Sample*> val = comparisons.at(i);
        int dist = get<0>(val)->dist(get<1>(val), cutoff);
        if(dist <= cutoff){
            mutex_lock.lock();
                fstream output("outputs/all.txt", fstream::app);
                output << get<0>(val)->uuid << " " << get<1>(val)->uuid << " " << dist << endl;
                output.close();
            mutex_lock.unlock();
        }
    }

    //Join the threads
    for(int i=0;i<threads.size();i++){
        threads.at(i).join();
    }

}

vector<Sample*> load_saves(){
    unordered_set<string> saves;
    for (const auto & entry : fs::directory_iterator("./saves")){
        string p = entry.path();
        //Check for .gitkeep or nothing (we want to ignore this)
        if(p == "./saves/.gitkeep" || p == ".saves/"){
            continue;
        }
        
        //Remove `.A` etc
        p.pop_back();
        p.pop_back();

        //Only add if the path found is not empty
        if(p.find_first_not_of(' ') != string::npos){
            saves.insert(p);
        }
    }

    vector<Sample*> samples;
    for(const string elem: saves){
        Sample *s = readSample(elem);
        samples.push_back(s);
    }

    return samples;
}

int main(int nargs, const char* args[]){
    int thread_count = 20;
    if(check_compute(nargs, args)){
        // compute(stoi(args[2]));
        vector<Sample*> samples = load_saves();
        compute_loaded(stoi(args[2]), samples, thread_count);
        return 0;
    }

    //Load reference
    string reference;
    char ch;
    fstream fin("ref.upper.fasta", fstream::in);
    while (fin >> noskipws >> ch) {
        reference += ch;
    }
    fin.close();

    fstream fin2("tb-exclude.txt", fstream::in);
    string acc;
    unordered_set<int> mask;
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
    
    if(check_bulk_load(nargs, args)){
        bulk_load(args[2], reference, mask, thread_count);
    }

    if(check_add_sample(nargs, args)){
        add_sample(args[2], reference, mask);
    }

}

