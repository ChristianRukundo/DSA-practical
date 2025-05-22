// LinkedList.h
// This header file defines a basic Singly Linked List data structure
// capable of storing 'Item' objects.
#pragma once // Ensures this header is included only once per compilation unit.

#include "Item.h"    // We need the definition of the 'Item' struct to store it in the list.
#include <vector>    // For std::vector, used in the toVector() method.
#include <iostream>  // For std::cout and std::endl, used in the optional display() method.
#include <stdexcept> // For std::runtime_error (optional, if adding error handling like throwing exceptions)

// Node structure for the Singly Linked List.
// Each node holds an 'Item' and a pointer to the next node in the list.
struct ListNode
{
    Item data;      // The data stored in this node (an Item object).
    ListNode *next; // Pointer to the next ListNode in the sequence, or nullptr if this is the last node.

    // Constructor for ListNode.
    // Initializes the node with the given item data and sets the next pointer to nullptr.
    ListNode(const Item &itemData) : data(itemData), next(nullptr) {}
};

// SinglyLinkedList class definition.
// Encapsulates the logic for managing a list of Items.
class SinglyLinkedList
{
private:
    ListNode *head; // Pointer to the first node (head) of the list. Nullptr if the list is empty.
    ListNode *tail; // Pointer to the last node (tail) of the list. Nullptr if empty. Used for O(1) appends.
    int count;      // Keeps track of the number of items currently in the list.

public:
    // Constructor: Initializes an empty linked list.
    SinglyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Destructor: Frees all dynamically allocated memory for the list nodes.
    // This is crucial to prevent memory leaks.
    ~SinglyLinkedList()
    {
        ListNode *current = head;
        while (current != nullptr)
        {
            ListNode *nodeToDelete = current; // Store the current node to delete it.
            current = current->next;          // Move to the next node *before* deleting the current one.
            delete nodeToDelete;              // Free the memory of the stored node.
        }
        // head and tail will be dangling pointers here, but the SinglyLinkedList object itself is being destroyed,
        // so these pointers will no longer be accessible.
    }

    // Adds an item to the end (tail) of the list.
    // Time Complexity: O(1) because we maintain a `tail` pointer.
    void append(const Item &item)
    {
        ListNode *newNode = new ListNode(item); // Dynamically allocate a new node.
                                                // `new` can throw std::bad_alloc if memory allocation fails.

        if (head == nullptr)
        {                   // If the list is currently empty.
            head = newNode; // The new node becomes both the head and the tail.
            tail = newNode;
        }
        else
        {                         // If the list is not empty.
            tail->next = newNode; // Link the current tail's `next` pointer to the new node.
            tail = newNode;       // Update the `tail` pointer to be the new node.
        }
        count++; // Increment the item count.
    }

    // Checks if the list is empty.
    // Time Complexity: O(1).
    bool isEmpty() const
    {                           // `const` because this method doesn't modify the list.
        return head == nullptr; // Or equivalently, `return count == 0;`
    }

    // Returns the number of items in the list.
    // Time Complexity: O(1).
    int size() const
    { // `const`
        return count;
    }

    // Converts the linked list to a std::vector<Item>.
    // This is useful for operations that are more efficient on vectors (like std::sort)
    // or when an API expects a vector.
    // Time Complexity: O(N), where N is the number of items in the list, as it iterates through all nodes.
    // Space Complexity: O(N) for the new std::vector created.
    std::vector<Item> toVector() const
    { // `const`
        std::vector<Item> vec;
        if (count > 0)
        {
            // Pre-allocate memory in the vector if the size is known.
            // This can improve performance by reducing reallocations if adding many elements.
            vec.reserve(count);
        }
        ListNode *current = head;
        while (current != nullptr)
        {
            vec.push_back(current->data); // Add a copy of the item's data to the vector.
            current = current->next;
        }
        return vec; // Return the newly created vector.
    }

    // (Optional) Displays the contents of the linked list to std::cout.
    // Useful for debugging. Uses the overloaded operator<< for Item.
    // Time Complexity: O(N).
    void display() const
    { // `const`
        ListNode *current = head;
        std::cout << "LinkedList Contents (Size: " << count << "):" << std::endl;
        int nodeNum = 0;
        while (current != nullptr)
        {
            std::cout << "  Node " << ++nodeNum << ": " << current->data; // Assumes Item has operator<<
            if (current->next != nullptr)
            {
                std::cout << " ->" << std::endl;
            }
            else
            {
                std::cout << " -> nullptr" << std::endl;
            }
            current = current->next;
        }
        if (head == nullptr)
        {
            std::cout << "  (empty list)" << std::endl;
        }
    }
};