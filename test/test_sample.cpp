#include <gtest/gtest.h>
#include <unordered_set>
#include "../src/include/sample.hpp"


/**
* @brief Print contents of a sample for inspection
*/
void print_sample(Sample* s){
    cout << "Sample GUID: " << s->uuid << endl;
    cout << "A: { ";
    for(const int elem : s->A){
        cout << elem << ", ";
    }
    cout << "}" << endl << endl;

    cout << "C: { ";
    for(const int elem : s->C){
        cout << elem << ", ";
    }
    cout << "}" << endl << endl;

    cout << "G: { ";
    for(const int elem : s->G){
        cout << elem << ", ";
    }
    cout << "}" << endl << endl;

    cout << "T: { ";
    for(const int elem : s->T){
        cout << elem << ", ";
    }
    cout << "}" << endl << endl;

    cout << "N: { ";
    for(const int elem : s->N){
        cout << elem << ", ";
    }
    cout << "}" << endl << endl;
}

/**
* @brief Testing that samples initalise properly
*/
TEST(sample, init){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    string expected_reference = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    EXPECT_EQ(expected_reference, reference);

    unordered_set<int> expected_mask = {1};
    EXPECT_EQ(expected_mask, mask);

    //Empty set for checking things which shouldn't have changed
    unordered_set<int> empty;

    //For simplicity here, check without a mask
    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, empty);
    unordered_set<int> C_1 = {0};
    EXPECT_EQ(s1->uuid, "uuid1");
    EXPECT_EQ(empty, s1->A);
    EXPECT_EQ(C_1, s1->C);
    EXPECT_EQ(empty, s1->G);
    EXPECT_EQ(empty, s1->T);
    EXPECT_EQ(empty, s1->N);

    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, empty);
    unordered_set<int> T_2 = {1};
    EXPECT_EQ(s2->uuid, "uuid2");
    EXPECT_EQ(empty, s2->A);
    EXPECT_EQ(empty, s2->C);
    EXPECT_EQ(empty, s2->G);
    EXPECT_EQ(T_2, s2->T);
    EXPECT_EQ(empty, s2->N);

    //Check that masking position 1 results in no entries for this
    s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    EXPECT_EQ(s2->uuid, "uuid2");
    EXPECT_EQ(empty, s2->A);
    EXPECT_EQ(empty, s2->C);
    EXPECT_EQ(empty, s2->G);
    EXPECT_EQ(empty, s2->T);
    EXPECT_EQ(empty, s2->N);

    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, empty);
    unordered_set<int> G_3 = {14};
    unordered_set<int> N_3 = {0};
    EXPECT_EQ(s3->uuid, "uuid3");
    EXPECT_EQ(empty, s3->A);
    EXPECT_EQ(empty, s3->C);
    EXPECT_EQ(G_3, s3->G);
    EXPECT_EQ(empty, s3->T);
    EXPECT_EQ(N_3, s3->N);

    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, empty);
    unordered_set<int> G_4 = {1};
    unordered_set<int> N_4 = {0};
    unordered_set<int> T_4 = {71};
    EXPECT_EQ(s4->uuid, "uuid4");
    EXPECT_EQ(empty, s4->A);
    EXPECT_EQ(empty, s4->C);
    EXPECT_EQ(G_4, s4->G);
    EXPECT_EQ(T_4, s4->T);
    EXPECT_EQ(N_4, s4->N);
}

/**
Dummy sample distance matrix (no mask/cutoff)
 |   1   2   3   4
 -------------------
1|   0   2   1   2
2|   2   0   2   2
3|   1   2   0   3
4|   2   2   3   0

*/

/**
* @brief Test distance calculations without mask or cutoff
*/
TEST(sample, dist_no_mask_no_cutoff){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> empty;

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, empty);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, empty);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, empty);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, empty);

    EXPECT_EQ(s1->dist(s1, 99999), 0);
    EXPECT_EQ(s1->dist(s2, 99999), 2);
    EXPECT_EQ(s1->dist(s3, 99999), 1);
    EXPECT_EQ(s1->dist(s4, 99999), 2);

    EXPECT_EQ(s2->dist(s1, 99999), 2);
    EXPECT_EQ(s2->dist(s2, 99999), 0);
    EXPECT_EQ(s2->dist(s3, 99999), 2);
    EXPECT_EQ(s2->dist(s4, 99999), 2);

    EXPECT_EQ(s3->dist(s1, 99999), 1);
    EXPECT_EQ(s3->dist(s2, 99999), 2);
    EXPECT_EQ(s3->dist(s3, 99999), 0);
    EXPECT_EQ(s3->dist(s4, 99999), 3);

    EXPECT_EQ(s4->dist(s1, 99999), 2);
    EXPECT_EQ(s4->dist(s2, 99999), 2);
    EXPECT_EQ(s4->dist(s3, 99999), 3);
    EXPECT_EQ(s4->dist(s4, 99999), 0);

}

/**
Dummy sample distance matrix (no cutoff)
 |   1   2   3   4
 -------------------
1|   0   1   1   1
2|   1   0   1   1
3|   1   1   0   2
4|   1   1   2   0

*/

/**
* @brief Test distance calculations without cutoff
*/
TEST(sample, dist_no_cutoff){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);

    EXPECT_EQ(s1->dist(s1, 99999), 0);
    EXPECT_EQ(s1->dist(s2, 99999), 1);
    EXPECT_EQ(s1->dist(s3, 99999), 1);
    EXPECT_EQ(s1->dist(s4, 99999), 1);

    EXPECT_EQ(s2->dist(s1, 99999), 1);
    EXPECT_EQ(s2->dist(s2, 99999), 0);
    EXPECT_EQ(s2->dist(s3, 99999), 1);
    EXPECT_EQ(s2->dist(s4, 99999), 1);

    EXPECT_EQ(s3->dist(s1, 99999), 1);
    EXPECT_EQ(s3->dist(s2, 99999), 1);
    EXPECT_EQ(s3->dist(s3, 99999), 0);
    EXPECT_EQ(s3->dist(s4, 99999), 2);

    EXPECT_EQ(s4->dist(s1, 99999), 1);
    EXPECT_EQ(s4->dist(s2, 99999), 1);
    EXPECT_EQ(s4->dist(s3, 99999), 2);
    EXPECT_EQ(s4->dist(s4, 99999), 0);

}

/**
* @brief Test distance calculations without mask, cutoff=2
*/
TEST(sample, dist_no_mask_cutoff_2){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> empty;

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, empty);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, empty);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, empty);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, empty);

    //Cutoff of 2 should actually behave the same as no cutoff here
    //If a cutoff is reached, the returned dist is cutoff +1
    //Cases which have dist > 2 are only ever 3, which is the returned value anyway
    EXPECT_EQ(s1->dist(s1, 2), 0);
    EXPECT_EQ(s1->dist(s2, 2), 2);
    EXPECT_EQ(s1->dist(s3, 2), 1);
    EXPECT_EQ(s1->dist(s4, 2), 2);

    EXPECT_EQ(s2->dist(s1, 2), 2);
    EXPECT_EQ(s2->dist(s2, 2), 0);
    EXPECT_EQ(s2->dist(s3, 2), 2);
    EXPECT_EQ(s2->dist(s4, 2), 2);

    EXPECT_EQ(s3->dist(s1, 2), 1);
    EXPECT_EQ(s3->dist(s2, 2), 2);
    EXPECT_EQ(s3->dist(s3, 2), 0);
    EXPECT_EQ(s3->dist(s4, 2), 3);

    EXPECT_EQ(s4->dist(s1, 2), 2);
    EXPECT_EQ(s4->dist(s2, 2), 2);
    EXPECT_EQ(s4->dist(s3, 2), 3);
    EXPECT_EQ(s4->dist(s4, 2), 0);

}

/**
* @brief Test distance calculations without mask, cutoff=1
*/
TEST(sample, dist_no_mask_cutoff_1){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> empty;

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, empty);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, empty);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, empty);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, empty);

    //Cutoff of 1 should now return some different distances
    EXPECT_EQ(s1->dist(s1, 1), 0);
    EXPECT_EQ(s1->dist(s2, 1), 2);
    EXPECT_EQ(s1->dist(s3, 1), 1);
    EXPECT_EQ(s1->dist(s4, 1), 2);

    EXPECT_EQ(s2->dist(s1, 1), 2);
    EXPECT_EQ(s2->dist(s2, 1), 0);
    EXPECT_EQ(s2->dist(s3, 1), 2);
    EXPECT_EQ(s2->dist(s4, 1), 2);

    EXPECT_EQ(s3->dist(s1, 1), 1);
    EXPECT_EQ(s3->dist(s2, 1), 2);
    EXPECT_EQ(s3->dist(s3, 1), 0);
    EXPECT_EQ(s3->dist(s4, 1), 2);

    EXPECT_EQ(s4->dist(s1, 1), 2);
    EXPECT_EQ(s4->dist(s2, 1), 2);
    EXPECT_EQ(s4->dist(s3, 1), 2);
    EXPECT_EQ(s4->dist(s4, 1), 0);

}

/**
* @brief Test distance calculations cutoff = 2
*/
TEST(sample, dist_cutoff_2){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);

    //Again, these values should not change due to cutoff

    EXPECT_EQ(s1->dist(s1, 2), 0);
    EXPECT_EQ(s1->dist(s2, 2), 1);
    EXPECT_EQ(s1->dist(s3, 2), 1);
    EXPECT_EQ(s1->dist(s4, 2), 1);

    EXPECT_EQ(s2->dist(s1, 2), 1);
    EXPECT_EQ(s2->dist(s2, 2), 0);
    EXPECT_EQ(s2->dist(s3, 2), 1);
    EXPECT_EQ(s2->dist(s4, 2), 1);

    EXPECT_EQ(s3->dist(s1, 2), 1);
    EXPECT_EQ(s3->dist(s2, 2), 1);
    EXPECT_EQ(s3->dist(s3, 2), 0);
    EXPECT_EQ(s3->dist(s4, 2), 2);

    EXPECT_EQ(s4->dist(s1, 2), 1);
    EXPECT_EQ(s4->dist(s2, 2), 1);
    EXPECT_EQ(s4->dist(s3, 2), 2);
    EXPECT_EQ(s4->dist(s4, 2), 0);

}

/**
* @brief Test distance calculations cutoff = 1
*/
TEST(sample, dist_cutoff_1){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);

    //Again, these values should not change due to cutoff

    EXPECT_EQ(s1->dist(s1, 1), 0);
    EXPECT_EQ(s1->dist(s2, 1), 1);
    EXPECT_EQ(s1->dist(s3, 1), 1);
    EXPECT_EQ(s1->dist(s4, 1), 1);

    EXPECT_EQ(s2->dist(s1, 1), 1);
    EXPECT_EQ(s2->dist(s2, 1), 0);
    EXPECT_EQ(s2->dist(s3, 1), 1);
    EXPECT_EQ(s2->dist(s4, 1), 1);

    EXPECT_EQ(s3->dist(s1, 1), 1);
    EXPECT_EQ(s3->dist(s2, 1), 1);
    EXPECT_EQ(s3->dist(s3, 1), 0);
    EXPECT_EQ(s3->dist(s4, 1), 2);

    EXPECT_EQ(s4->dist(s1, 1), 1);
    EXPECT_EQ(s4->dist(s2, 1), 1);
    EXPECT_EQ(s4->dist(s3, 1), 2);
    EXPECT_EQ(s4->dist(s4, 1), 0);

}

/**
* @brief Test distance calculations cutoff = 0. This is a ridiculous test case (cutoff of 0 makes all of this pointless), but checks it works
*/
TEST(sample, dist_cutoff_0){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);

    EXPECT_EQ(s1->dist(s1, 0), 0);
    EXPECT_EQ(s1->dist(s2, 0), 1);
    EXPECT_EQ(s1->dist(s3, 0), 1);
    EXPECT_EQ(s1->dist(s4, 0), 1);

    EXPECT_EQ(s2->dist(s1, 0), 1);
    EXPECT_EQ(s2->dist(s2, 0), 0);
    EXPECT_EQ(s2->dist(s3, 0), 1);
    EXPECT_EQ(s2->dist(s4, 0), 1);

    EXPECT_EQ(s3->dist(s1, 0), 1);
    EXPECT_EQ(s3->dist(s2, 0), 1);
    EXPECT_EQ(s3->dist(s3, 0), 0);
    EXPECT_EQ(s3->dist(s4, 0), 1);

    EXPECT_EQ(s4->dist(s1, 0), 1);
    EXPECT_EQ(s4->dist(s2, 0), 1);
    EXPECT_EQ(s4->dist(s3, 0), 1);
    EXPECT_EQ(s4->dist(s4, 0), 0);

}

/**
* @brief Test distance calculations for a new sample which is pretty far away from the others
*/
TEST(sample, dist_new){
    string reference = load_reference("cases/dummy/reference.fasta");
    unordered_set<int> mask = load_mask("cases/dummy/mask.txt");

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    //Order shouldn't matter here
    EXPECT_EQ(s1->dist(s5, 99999), s5->dist(s1, 99999));
    EXPECT_EQ(s2->dist(s5, 99999), s5->dist(s2, 99999));
    EXPECT_EQ(s3->dist(s5, 99999), s5->dist(s3, 99999));
    EXPECT_EQ(s4->dist(s5, 99999), s5->dist(s4, 99999));

    //This is far away
    EXPECT_EQ(s1->dist(s5, 99999), 79);
    EXPECT_EQ(s2->dist(s5, 99999), 79);
    EXPECT_EQ(s3->dist(s5, 99999), 77);
    EXPECT_EQ(s4->dist(s5, 99999), 78);

    //Check that cutoffs work here too
    EXPECT_EQ(s1->dist(s5, 70), 71);
    EXPECT_EQ(s2->dist(s5, 70), 71);
    EXPECT_EQ(s3->dist(s5, 70), 71);
    EXPECT_EQ(s4->dist(s5, 70), 71);

    EXPECT_EQ(s1->dist(s5, 20), 21);
    EXPECT_EQ(s2->dist(s5, 20), 21);
    EXPECT_EQ(s3->dist(s5, 20), 21);
    EXPECT_EQ(s4->dist(s5, 20), 21);

}

/**
* @brief Double check that using any complete genome as a reference doesn't impact distances
*/
TEST(sample, change_reference){
    string reference = load_reference("cases/dummy/reference.fasta");
    string reference2 = load_reference("cases/dummy/1.fasta");
    unordered_set<int> mask;

    Sample* s1 = new Sample("cases/dummy/1.fasta", reference, mask);
    Sample* s2 = new Sample("cases/dummy/2.fasta", reference, mask);
    Sample* s3 = new Sample("cases/dummy/3.fasta", reference, mask);
    Sample* s4 = new Sample("cases/dummy/4.fasta", reference, mask);
    Sample* s5 = new Sample("cases/dummy/5.fasta", reference, mask);

    Sample* s1_2 = new Sample("cases/dummy/1.fasta", reference2, mask);
    Sample* s2_2 = new Sample("cases/dummy/2.fasta", reference2, mask);
    Sample* s3_2 = new Sample("cases/dummy/3.fasta", reference2, mask);
    Sample* s4_2 = new Sample("cases/dummy/4.fasta", reference2, mask);
    Sample* s5_2 = new Sample("cases/dummy/5.fasta", reference2, mask);

    EXPECT_EQ(s1->dist(s1, 99999), s1_2->dist(s1_2, 99999));
    EXPECT_EQ(s1->dist(s2, 99999), s1_2->dist(s2_2, 99999));
    EXPECT_EQ(s1->dist(s3, 99999), s1_2->dist(s3_2, 99999));
    EXPECT_EQ(s1->dist(s4, 99999), s1_2->dist(s4_2, 99999));
    EXPECT_EQ(s1->dist(s5, 99999), s1_2->dist(s5_2, 99999));

    EXPECT_EQ(s2->dist(s1, 99999), s2_2->dist(s1_2, 99999));
    EXPECT_EQ(s2->dist(s2, 99999), s2_2->dist(s2_2, 99999));
    EXPECT_EQ(s2->dist(s3, 99999), s2_2->dist(s3_2, 99999));
    EXPECT_EQ(s2->dist(s4, 99999), s2_2->dist(s4_2, 99999));
    EXPECT_EQ(s2->dist(s5, 99999), s2_2->dist(s5_2, 99999));

    EXPECT_EQ(s3->dist(s1, 99999), s3_2->dist(s1_2, 99999));
    EXPECT_EQ(s3->dist(s2, 99999), s3_2->dist(s2_2, 99999));
    EXPECT_EQ(s3->dist(s3, 99999), s3_2->dist(s3_2, 99999));
    EXPECT_EQ(s3->dist(s4, 99999), s3_2->dist(s4_2, 99999));
    EXPECT_EQ(s3->dist(s5, 99999), s3_2->dist(s5_2, 99999));

    EXPECT_EQ(s4->dist(s1, 99999), s4_2->dist(s1_2, 99999));
    EXPECT_EQ(s4->dist(s2, 99999), s4_2->dist(s2_2, 99999));
    EXPECT_EQ(s4->dist(s3, 99999), s4_2->dist(s3_2, 99999));
    EXPECT_EQ(s4->dist(s4, 99999), s4_2->dist(s4_2, 99999));
    EXPECT_EQ(s4->dist(s5, 99999), s4_2->dist(s5_2, 99999));

    EXPECT_EQ(s5->dist(s1, 99999), s5_2->dist(s1_2, 99999));
    EXPECT_EQ(s5->dist(s2, 99999), s5_2->dist(s2_2, 99999));
    EXPECT_EQ(s5->dist(s3, 99999), s5_2->dist(s3_2, 99999));
    EXPECT_EQ(s5->dist(s4, 99999), s5_2->dist(s4_2, 99999));
    EXPECT_EQ(s5->dist(s5, 99999), s5_2->dist(s5_2, 99999));
}

