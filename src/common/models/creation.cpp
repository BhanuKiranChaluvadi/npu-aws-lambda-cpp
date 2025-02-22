#include "creation.hpp"
#include <aws/core/utils/UUID.h>
#include <aws/core/utils/DateTime.h>

void Creation::generate_id()
{
    creation_id = Aws::Utils::UUID::RandomUUID();
    creation_date = Aws::Utils::DateTime::Now().ToGmtString(
        Aws::Utils::DateFormat::ISO_8601);
}

bool Creation::validate() const
{
    return !element_name.empty() &&
           !title.empty() &&
           !image_data.empty() &&
           !user_id.empty();
}