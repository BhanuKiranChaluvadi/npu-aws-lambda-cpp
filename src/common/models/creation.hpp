#pragma once
#include <string>
#include <vector>

struct Creation
{
    std::string creation_id;
    std::string user_id;
    std::string element_name;
    std::string title;
    std::string description;
    std::string image_data;
    std::string image_key;
    std::string thumbnail_key;
    std::vector<std::string> tags;
    std::string creation_date;

    void generate_id();
    bool validate() const; // Declaration
};