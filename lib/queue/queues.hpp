#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <string>
#include <vector>

enum class queues
{
    fav,
    boost,
    post
};

void enqueue(queues toenqueue, const std::string& account, const std::vector<std::string>& add);
void dequeue(queues todequeue, const std::string& account, const std::vector<std::string>& remove);

#endif