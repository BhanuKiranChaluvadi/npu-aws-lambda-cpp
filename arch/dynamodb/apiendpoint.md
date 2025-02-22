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

## Global Secondary Index Attributes for NPU Platform

### Complete GSI Definition
```json
{
  "GlobalSecondaryIndexes": [
    {
      "IndexName": "ElementNameIndex",
      "KeySchema": [
        {
          "AttributeName": "element_name",
          "KeyType": "HASH"
        },
        {
          "AttributeName": "creation_date",
          "KeyType": "RANGE"  // Optional sort key for chronological ordering
        }
      ],
      "Projection": {
        "ProjectionType": "ALL"  // Copies all attributes to the index
      },
      "ProvisionedThroughput": {
        "ReadCapacityUnits": 5,
        "WriteCapacityUnits": 5
      }
    }
  ]
}
```

### Key Components

#### 1. Key Schema
- **Partition Key** (`element_name`): Primary lookup attribute
- **Sort Key** (`creation_date`): Optional, enables range queries within an `element_name`

#### 2. Projection Type
Determines which attributes are copied to the index:
- **ALL**: All attributes from main table
- **KEYS_ONLY**: Only the index and primary keys
- **INCLUDE**: Specific attributes you list

#### 3. Provisioned Throughput
Defines read/write capacity for the index

### Example Query Using GSI
```cpp
// Search for recent creations using a specific element
QueryRequest()
  .WithIndexName("ElementNameIndex")
  .WithKeyConditionExpression(
    "element_name = :val AND creation_date > :date"
  )
  .WithExpressionAttributeValues({
    {":val", "brick-2x4"},
    {":date", "2024-01-01"}
  });
```

The GSI essentially creates an efficient lookup table optimized for element-based searches, which is a core feature of the NPU platform.

