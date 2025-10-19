#include<iostream>
#include<vector>

struct DataFrame
{
    // Filter()
    
    // BinOps()
};


std::vector<long long> getFeature(long long price_min, long long price_max) {

}

using namespace std; 
int main() {
    
    int price_min, price_max;
    cin >> price_min >> price_max;

    auto feature_vector = getFeature(price_min, price_max);

    for (auto &val : feature_vector) {
        cout << val << endl;
    }

    return 0;
}