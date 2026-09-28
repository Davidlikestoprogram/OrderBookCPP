#include <cstdint>
#include <map>
#include <list>
#include <functional>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <vector>

using namespace std;

enum class Side : uint8_t {
    BUY,
    SELL,
    LIMIT_BUY,
    LIMIT_SELL
};

struct Order {
    uint64_t id;
    uint64_t end_id;
    Side side;
    uint64_t price;
    uint32_t quantity;
};


class OrderBook {

    public: 
        map<uint64_t, list<Order>,greater<uint64_t>> bid;
        map<uint64_t, list<Order>,less<uint64_t>> ask;
        
        void limit_buy(uint64_t id, uint64_t max_price, uint32_t quantity){
            Order incomming {id,0,Side::LIMIT_BUY,max_price,quantity};
        

            while (ask.empty() == false&&incomming.quantity>0 && ask.begin()->first <=  max_price ){
                auto& best_ask = ask.begin()->second;
                auto& fill_order = best_ask.front();
                uint32_t fill_amount = min(fill_order.quantity, incomming.quantity);

                incomming.quantity -= fill_amount;
                fill_order.quantity -= fill_amount;
                incomming.end_id = fill_order.id;

                if (fill_order.quantity == 0){
                    best_ask.pop_front();

                    if(best_ask.empty() == true){
                        ask.erase(ask.begin());
                    }
                }
            }
            if (incomming.quantity >0){
                bid[max_price].push_back(incomming);
            }
        }
        void buy(uint64_t id, uint32_t quantity){
            Order incomming {id,0,Side::BUY,0,quantity};
        

            while (ask.empty() == false&&incomming.quantity>0 ){
                auto& best_ask = ask.begin()->second;
                auto& fill_order = best_ask.front();
                uint32_t fill_amount = min(fill_order.quantity, incomming.quantity);

                incomming.quantity -= fill_amount;
                fill_order.quantity -= fill_amount;
                incomming.end_id = fill_order.id;

                if (fill_order.quantity == 0){
                    best_ask.pop_front();

                    if(best_ask.empty() == true){
                        ask.erase(ask.begin());
                    }
                }
            }
            if (incomming.quantity >0){
                cout << incomming.quantity<< "dropped due to lack of liquidity";
            }
        }

        void limit_sell(uint64_t id, uint64_t min_price, uint32_t quantity){
            Order incomming {id,0,Side::LIMIT_SELL,min_price,quantity};
        

            while (bid.empty() == false&&incomming.quantity>0 && bid.begin()->first >=  min_price){
                auto& best_bid = bid.begin()->second;
                auto& fill_order = best_bid.front();
                uint32_t fill_amount = min(fill_order.quantity, incomming.quantity);

                incomming.quantity -= fill_amount;
                fill_order.quantity -= fill_amount;
                incomming.end_id = fill_order.id;

                if (fill_order.quantity == 0){
                    best_bid.pop_front();

                    if(best_bid.empty() == true){
                        bid.erase(bid.begin());
                    }
                }
            }
            if (incomming.quantity >0){
                ask[min_price].push_back(incomming);
            }
        }

        void sell(uint64_t id, uint32_t quantity){
            Order incomming {id,0,Side::SELL,0,quantity};
        

            while (bid.empty() == false&&incomming.quantity>0){

                auto& best_bid = bid.begin()->second;
                auto& fill_order = best_bid.front();
                uint32_t fill_amount = min(fill_order.quantity, incomming.quantity);

                incomming.quantity -= fill_amount;
                fill_order.quantity -= fill_amount;
                incomming.end_id = fill_order.id;

                if (fill_order.quantity == 0){
                    best_bid.pop_front();

                    if(best_bid.empty() == true){
                        bid.erase(bid.begin());
                    }
                }
            }
            if (incomming.quantity >0){
                cout << incomming.quantity<< "dropped due to lack of liquidity";
            }
        
        }

        void vis_orderbook(){
            map<double, uint64_t> ask_book;
            for (auto it = ask.rbegin(); it != ask.rend(); ++it){
                double price = static_cast<double>(it->first)/100;
                auto& order_list = it->second;

                int total_quantity = 0;
                for (auto it2 = order_list.begin(); it2 != order_list.end(); ++it2) {
                    total_quantity+= it2->quantity;
                }
                ask_book[price] = total_quantity;
            }

            for (auto it = ask_book.rbegin(); it != ask_book.rend(); ++it){
                cout << it->first << "->" << it->second<<endl;
            }

            map<double, uint64_t,greater<double>> bid_book;
            for (auto it = bid.begin(); it != bid.end(); ++it){
                double price = static_cast<double>(it->first)/100;
                auto& order_list = it->second;

                int total_quantity = 0;
                for (auto it2 = order_list.begin(); it2 != order_list.end(); ++it2) {
                    total_quantity+= it2->quantity;
                }
                bid_book[price] = total_quantity;
            }
            for (auto it = bid_book.begin(); it != bid_book.end(); ++it){
                cout << it->first << "->" << it->second<<endl;
            }

            if (!ask.empty() && !bid.empty()) {

                double best_ask = static_cast<double>(ask.begin()->first)/100;
                double best_bid = static_cast<double>(bid.begin()->first)/100;

                double spread = best_ask - best_bid;
                double absolute_spread = best_ask - best_bid;

                double mid_price = (best_ask + best_bid) / 2.0;

                cout<< "Bid Ask Spread Percentage" <<(absolute_spread / mid_price) * 100.0;
                std::cout<< "Spread:" << std::fixed << spread<< endl;
            }


        }

        


};

int main() {
    OrderBook book;
    int NUM_ORDERS = 500000;

    // comment out any outputs for testing limit orders
    // only works for orders in cents
    // CANCEL 


    cout << "Starting benchmark with " << NUM_ORDERS * 2 << " total operations...\n";
    auto start_time = chrono::high_resolution_clock::now();

    for (uint64_t i = 1; i <= NUM_ORDERS; ++i) {
        book.limit_sell(i, 100 + (i % 100), 10);
    }

    for (uint64_t i = NUM_ORDERS + 1; i <= NUM_ORDERS * 2; ++i) {
        book.buy(i, 10);
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto total_dur_us = chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    double total_dur_ms = total_dur_us / 1000;
    
    int total_ops = NUM_ORDERS * 2;
    double ops_per_sec = (total_ops / static_cast<double>(total_dur_us))*1000000;
    double avg_latency_ns = (static_cast<double>(total_dur_us) * 1000) / total_ops;


    cout << "Total Operations:" << total_ops << endl;
    cout << "Total Time Elapsed:" << total_dur_ms << " ms "<<endl;
    cout << "Throughput: " << static_cast<uint64_t>(ops_per_sec) << " ops/sec"<< endl;
    cout << "Average Latency: " << avg_latency_ns << " ns / order"<<endl;

    return 0;
}