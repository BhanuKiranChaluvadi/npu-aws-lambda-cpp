# NPUCreations Database Schema (DynamoDB)

## 1. Main Table Overview
The **NPUCreations** table serves as the primary database for storing all LEGO creations. It functions like a spreadsheet where each row represents a unique LEGO creation.

## 2. Key Schema
DynamoDB uniquely identifies each item (row) in the table using a **Key Schema** consisting of two components:

- **Partition Key (`creation_id`)**: The primary identifier that determines where the data is stored.
- **Sort Key (`user_id`)** (Optional): A secondary key that enables sorting within a partition.

### Example Item:
```json
{
    "creation_id": "12345",  // Partition Key
    "user_id": "user789"     // Sort Key
}
```

## 3. Attributes
Attributes are additional data fields that each item can contain. Since DynamoDB is schema-less, each item can have different attributes.

### Example Item with Attributes:
```json
{
    "title": "Cool Spaceship",
    "description": "Made using bricks",
    "image_url": "https://...",
    "creation_date": "2025-02-22"
}
```

## 4. Global Secondary Index (GSI)
A **Global Secondary Index (GSI)** is like a copy of your table that allows efficient queries using different attributes. In this case, the **ElementNameIndex** enables searching creations by element name.

### Example Query:
> "Show all creations using a 2x4 brick"

## 5. Table Structure
### **Main Table Schema:**
| `creation_id` | `user_id`  | `element_name` | `title`       | `image_url`  |
|--------------|-----------|---------------|--------------|--------------|
| create123    | user789   | brick-2x4     | Cool Ship    | http://...   |
| create456    | user555   | plate-1x2     | Nice House   | http://...   |

- **`creation_id`** → Partition Key
- **`user_id`** → Sort Key
- **`element_name`, `title`, `image_url`** → Regular Attributes

### **Global Secondary Index (ElementNameIndex):**
| `element_name` | `creation_id` | `other_data` |
|--------------|--------------|--------------|
| brick-2x4   | create123   | ...          |
| plate-1x2   | create456   | ...          |

- **`element_name`** → New Partition Key
- **`creation_id`** → New Sort Key

## 6. Purpose of ElementNameIndex GSI

# Role of Global Secondary Index in NPU Platform

## Problem It Solves
From the case requirements:
> "They can also search for ideas and inspiration based on an element name among the uploaded NPU creations"

Without GSI:
- To find creations by element_name, we'd need to scan the entire table
- This would be slow and expensive for large datasets
- Not practical for real-time searches

## How ElementNameIndex Works

```json
{
  "IndexName": "ElementNameIndex",
  "KeySchema": [
    {
      "AttributeName": "element_name",
      "KeyType": "HASH"
    }
  ]
}
```

### Example queries it enables:
```cpp
// Find all creations using "brick-2x4"
QueryRequest()
  .WithIndexName("ElementNameIndex")
  .WithKeyConditionExpression("element_name = :val")
  .WithExpressionAttributeValues({":val", "brick-2x4"});
```

### Benefits for NPU Platform
**Fast Searches:**
- Users can quickly find inspirational uses of specific parts
- Supports the `GET /elements/{element_name}/creations` endpoint

**Efficient Queries:**
- Instead of scanning all creations
- Direct lookup by `element_name`

**Cost Effective:**
- Reduces DynamoDB read capacity usage
- Better performance/cost ratio for element-based searches

## 7. Attribute Definitions in DynamoDB

You don't need to specify attributes like `description`, `image_url`, etc., in the **AttributeDefinitions** section of your DynamoDB table creation. Here's why:

In DynamoDB, you only need to define attributes in `AttributeDefinitions` if they are used as:
- **Primary Key** (Partition/Hash key or Sort/Range key)
- **Secondary Index Keys** (GSI or LSI keys)

### Example Table Definition:
```json
{
    "TableName": "NPUCreations",
    "AttributeDefinitions": [
        {
            "AttributeName": "creation_id",    // Used in primary key
            "AttributeType": "S"
        },
        {
            "AttributeName": "user_id",        // Used in primary key
            "AttributeType": "S"
        },
        {
            "AttributeName": "element_name",   // Used in GSI
            "AttributeType": "S"
        },
        {
            "AttributeName": "creation_date",  // Used in GSI
            "AttributeType": "S"
        }
    ]
    // ...rest of the table definition...
}
```

### Key Points:
- **DynamoDB is schema-less** (except for key attributes)
- Regular attributes (`description`, `image_url`, etc.) can be added/removed freely when inserting items
- You can have different attributes for different items in the same table
- **Only key-related attributes need to be defined** in table creation

### Example Item Insert:
```json
{
    "creation_id": {"S": "create123"},     // Required (primary key)
    "user_id": {"S": "user789"},          // Required (sort key)
    "element_name": {"S": "brick-2x4"},   // Required (GSI key)
    "description": {"S": "Cool build"},   // Optional
    "image_url": {"S": "http://..."},     // Optional
    "tags": {"L": [{"S": "spaceship"}]}   // Optional
}
```

This approach allows for flexible data management while keeping queries efficient and cost-effective.

# Understanding DynamoDB Provisioned Throughput

## What is Provisioned Throughput?
```json
"ProvisionedThroughput": {
    "ReadCapacityUnits": 5,    // 5 RCU
    "WriteCapacityUnits": 5     // 5 WCU
}
```

## Capacity Units Explained
1. **Read Capacity Unit (RCU = 5)**
   - 1 RCU = 2 strongly consistent reads per second for items up to 4KB
   - 1 RCU = 4 eventually consistent reads per second for items up to 4KB
   - With 5 RCUs you get:
     - 10 strongly consistent reads/second
     - OR 20 eventually consistent reads/second

2. **Write Capacity Unit (WCU = 5)**
   - 1 WCU = 1 write per second for items up to 1KB
   - With 5 WCUs you get:
     - 5 writes per second for items up to 1KB each

## Cost Implications
- You pay for provisioned capacity whether you use it or not
- For development/testing, 5 units is usually sufficient
- For production, monitor usage and adjust as needed

## Alternative
Consider using `BillingMode: "PAY_PER_REQUEST"` for:
- Unpredictable workloads
- Low-traffic tables
- Development/testing environments
```json
{
    "TableName": "NPUCreations",
    "BillingMode": "PAY_PER_REQUEST",
    // Remove ProvisionedThroughput
    ...
}
```