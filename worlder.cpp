#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <set>

using namespace std;

map<char, double> letter_frequency = {
    {'e', 1},
    {'a', 0.75},
    {'r', 0.68},
    {'i', 0.67},
    {'o', 0.63},
    {'t', 0.61},
    {'n', 0.6},
    {'s', 0.51},
    {'l', 0.49},
    {'c', 0.40},
    {'u', 0.32},
    {'d', 0.3},
    {'p', 0.28},
    {'m', 0.26},
    {'h', 0.26},
    {'g', 0.23},
    {'b', 0.19},
    {'f', 0.16},
    {'y', 0.16},
    {'w', 0.12},
    {'k', 0.11},
    {'v', 0.09},
    {'x', 0.05},
    {'z', 0.05},
    {'j', 0.05},
    {'q', 0.05},
}; 

// my random word milestones: insurance, weird, candid, paltry
vector<float> word_frequency_cutoffs = {193271293/(double)23135851162, 11556427/(double)23135851162, 3066199/(double)23135851162, 263983/(double)23135851162};

bool IsNumber(const std::string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void GetAllWords(map<string, float> &all_words) {
    // retrieves words

    ifstream in("unigram_freq.csv");

    if (!in) {
        cerr << "Cannot open file";
    }

    string key, val;
    getline(in, key);
    float max_freq = -1;
    while (getline(in, key, ',')) {
        getline(in, val, '\n');
        
        // set max freq
        if (max_freq == -1) {
            max_freq = stof(val);
        }

        all_words[key] = stof(val) / max_freq;
    }
}

void FilterWords(map<string, float> &words, string current_criteria, string not_placed, string not_included) {
    map<string, float> temp_words;
    for (auto &word : words) {
        // length
        if (word.first.length() != current_criteria.length()) {
            continue;
        }

        bool breaks_criteria = false;
        string np = not_placed;
        for (int i = 0; i < word.first.length(); i++) {
            if (breaks_criteria) {
                break;
            }
            // current criteria
            if (current_criteria[i] != '-' && word.first[i] != current_criteria[i]) {
                breaks_criteria = true;
            }
            // remove letter from np if we are on a dash and it exists
            if (current_criteria[i] == '-') {
                auto pos = np.find(word.first[i]);
                if (pos != std::string::npos)
                    np.erase(pos, 1);
            }
            // includes a forbidden letter (skip placed forbidden letters)
            // TODO: allow unplaced letters as well
            if (current_criteria[i] == '-' && not_included.find(word.first[i]) != string::npos) {
                breaks_criteria = true;
            }
        }
        // if contains_copy is not empty, the word does not contain required letters
        if (np.length() > 0) {
            breaks_criteria = true;
        }
        if (breaks_criteria) {
            continue;
        }

        // add word
        temp_words[word.first] = word.second;
    }
    words = temp_words;
}

vector<pair<string, float>> ChooseBestWords(map<string, float> &words) {
    vector<pair<string, float>> best_words;
    float worst_score = -5000;

    map<string, float>::iterator it;
    for (it = words.begin(); it != words.end(); it++) {
        /*
        Calculate points (more points is better, each factor should be in the range of 0 ~ length of word):
          frequency of word: separate into five buckets, least common --> (0~1 * n) --> most common
          frequency of letters: sum up all frequency values (0~1 * n)
          duplicate letter penalty: subtract the frequency value of any repeated letter (counts less than the least frequent letter)
        */
        float score = 0;
        int word_length = it->first.size();
        
        // word frequency
        // determine which bucket it is in: word_frequency_cutoffs
        // TODO: calculate fractional value
        if (it->second > word_frequency_cutoffs[0]) {
            score += 1 * word_length;
        } else if (it->second > word_frequency_cutoffs[1]) {
            score += 0.8 * word_length;
        } else if (it->second > word_frequency_cutoffs[2]) {
            score += 0.6 * word_length;
        } else if (it->second > word_frequency_cutoffs[3]) {
            score += 0.4 * word_length;
        } else {
            score += 0.2 * word_length;
        }

        // TODO: frequency of letters & duplicate letter penalties
        set<char> seen;
        for (int i = 0; i < it->first.size(); i++) {
            char letter = it->first[i];
            if (seen.find(letter) != seen.end()) {
                score += letter_frequency[letter];
            } else {
                score -= letter_frequency[letter];
            }
        }

        // add pair to best_words if better than worst score (and pop lowest if already at max size)
        if (score > worst_score) {
            if (best_words.size() >= 5) {
                // find lowest and second lowest
                int min_element_index = 0;
                float lowest_score = best_words[0].second;
                for (int i = 1; i < best_words.size(); i++) {
                    if (best_words[i].second < lowest_score) {
                        min_element_index = i;
                        lowest_score = best_words[i].second;
                    }
                }
                // remove lowest
                best_words.erase(best_words.begin() + min_element_index);
                // update lowest score
                float new_lowest_score = best_words[0].second;
                for (int i = 1; i < best_words.size(); i++) {
                    if (best_words[i].second < lowest_score) {
                        new_lowest_score = best_words[0].second;
                    }
                }
                worst_score = new_lowest_score;
            }
            // add new best word
            best_words.push_back(pair(it->first, score));
        }
    }

    return best_words;
}

bool GetInput(string &not_placed, string &not_included, string &criteria) {

    return false;
}

int main() {
    map<string, float> all_words;

    // get all words
    GetAllWords(all_words);

    while (true) {

        cout << endl << endl << "STARTING NEW GAME:" << endl;

        map<string, float> words = all_words;

        // 3 pieces of information
        string criteria = "";  // __a_re
        string not_placed = "";  // gh
        string not_included = "";  // zxqru

        vector<pair<string, float>> best_words;

        while (true) {
            // TODO: MOVE ALL INPUT!! wasn't working... 
            // input current criteria
            string new_current_criteria;
            cout << "CURRENT INFO ('q' to quit, 'u' or '-' if unchanged, digit for smart-start): ";
            cin >> new_current_criteria;

            // smart-start (enter '7' to kick off blank seven letter word)
            if (IsNumber(new_current_criteria)) {
                new_current_criteria = string(stoi(new_current_criteria), '-');
                not_placed = "-";
                not_included = "-";
            } else if (new_current_criteria == "q") {
                break;
            } else {
                cout << "NOT PLACED: ";
                cin >> not_placed;
                cout << "NOT INCLUDED: ";
                cin >> not_included;
            }

            if (new_current_criteria != "u" && new_current_criteria != "-") {
                criteria = new_current_criteria;
            }

            // update new info
            if (not_placed == "-") {
                not_placed = "";
            }
            if (not_included == "-") {
                not_included = "";
            }

            // filter out criteria
            FilterWords(words, criteria, not_placed, not_included);

            // choose best guess for remaining (point system)
            best_words = ChooseBestWords(words);

            // output results
            cout << criteria << " | " << not_placed << " | " << not_included << endl;
            cout << "THERE ARE " << words.size() << " REMAINING WORDS." << endl;
            cout << "TRY:" << endl;
            for (pair<string, float> p : best_words) {
                cout << p.first << " - " << p.second << endl;
            }
        }

    }

    return 0;
}
