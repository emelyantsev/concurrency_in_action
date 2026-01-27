#pragma once

#include <atomic>
#include <memory>


template<typename T>
class LockFreeStack {

    private:

        struct Node {

            std::shared_ptr<T> data;
            std::shared_ptr<Node> next;

            Node(T const& data_) : data( std::make_shared<T>(data_) ) {}
        };

        std::atomic< std::shared_ptr<Node> > head;

    public:

        void push( T const& data ) {

            auto new_node = std::make_shared<Node>( data );
            new_node->next = head.load();

            while ( !head.compare_exchange_weak( new_node->next, new_node ) );
        }

        std::shared_ptr<T> pop() {

            std::shared_ptr<Node> old_head = head.load();

            while ( old_head && !head.compare_exchange_weak( old_head, old_head->next ) );

            return old_head ? old_head->data : std::shared_ptr<T>();
        }
};