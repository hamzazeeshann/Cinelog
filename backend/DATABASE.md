# Cinelog Database - Technical Documentation

## Project Structure

```
backend/
├── include/
│   ├── models/          # Data schemas and structures
│   │   └── FilmLog.h    # Film log entry schema
│   ├── ds/              # Low-level data structures
│   │   ├── BTree.h      # B-Tree implementation (Order 5)
│   │   └── HashMap.h    # Custom dynamic hash map
│   └── core/            # Engine logic
│       └── CinelogDB.h  # Main database engine
├── src/
│   └── main.cpp         # Application entry point
└── tree.bin             # Persistent B-Tree storage (generated at runtime)
```

## Custom Data Structures

### 1. B-Tree (`include/ds/BTree.h`)

**Properties:**
- Order: 5 (max 4 keys per node)
- Primary index on `logId` (integer)
- Persistent storage via binary serialization
- Self-balancing on insert

**Operations:**
- `insert(FilmLog)`: O(log n) insertion
- `search(logId)`: O(log n) lookup
- `deleteRecord(logId)`: O(log n) deletion
- `updateRecord(logId, FilmLog)`: O(log n) update
- `getAllRecords()`: O(n) full scan

**Storage Format:**
- File header: 16 bytes (rootPos, nextPos)
- Node size: ~600 bytes (fixed per node)
- Direct memory serialization (no external deps)

### 2. HashMap (`include/ds/HashMap.h`)

**Properties:**
- Template-based: `HashMap<Key, Value>`
- Collision strategy: Linked list chaining
- Dynamic resizing: Doubles capacity when load factor > 0.7
- Manual memory management (`new`/`delete`)

**Hash Function:**
- DJB2 algorithm: `hash = (hash << 5) + hash + c`
- Modulo table size for bucket index

**Operations:**
- `insert(key, value)`: O(1) average, triggers resize if needed
- `get(key)`: O(1) average, returns pointer to value
- `find(key, result)`: O(1) average, returns boolean
- `remove(key)`: O(1) average
- `clear()`: O(n) cleanup

**Memory Management:**
- Each bucket head stored in dynamic array
- Chain nodes allocated individually
- Full cleanup in destructor
- Rehashing on resize (all nodes re-inserted)

### 3. FilmLog (`include/models/FilmLog.h`)

**Schema:**
```cpp
struct FilmLog {
    int logId;              // Unique identifier
    char username[32];      // Fixed-size username
    char movieTitle[64];    // Fixed-size movie title
    float rating;           // 0.0 - 5.0
    long timestamp;         // Unix timestamp
}
```

**Design Rationale:**
- Fixed-size fields for predictable serialization
- Direct `memcpy` serialization (no parsing overhead)
- 128 bytes total (cache-friendly)

## CinelogDB Public API

### Core Operations

#### `CinelogDB(const string& dbFile)`
Initializes database. Opens or creates B-Tree file. Rebuilds hash index from disk.

#### `int addLog(const char* username, const char* movieTitle, float rating)`
Adds new film log. Returns assigned `logId`. Updates both B-Tree and hash index.

**Example:**
```cpp
int id = db.addLog("alice", "The Matrix", 4.5);
```

#### `bool getLog(int logId, FilmLog& result)`
Retrieves log by ID. Returns `true` if found, populates `result`.

**Example:**
```cpp
FilmLog log;
if (db.getLog(42, log)) {
    // Use log.movieTitle, log.rating, etc.
}
```

#### `bool updateLog(int logId, const char* movieTitle, float rating)`
Updates existing log. Pass `nullptr` for movieTitle or `-1` for rating to skip field.

**Example:**
```cpp
db.updateLog(42, "The Matrix Reloaded", 4.0);
```

#### `bool deleteLog(int logId)`
Deletes log from both B-Tree and hash index. Returns `false` if not found.

**Example:**
```cpp
if (db.deleteLog(42)) {
    cout << "Deleted successfully";
}
```

#### `void deleteAll()`
**HARD RESET.** Deletes `tree.bin` file. Clears hash map. Resets ID counter to 1.

**Warning:** Cannot be undone.

**Example:**
```cpp
db.deleteAll();  // Entire database wiped
```

### Query Operations

#### `vector<FilmLog> searchByTitle(const char* substring)`
Linear scan of B-Tree. Returns all logs where `movieTitle` contains `substring`.

**Complexity:** O(n) where n = total logs

**Example:**
```cpp
vector<FilmLog> results = db.searchByTitle("Matrix");
// Returns: The Matrix, The Matrix Reloaded, etc.
```

#### `vector<FilmLog> getLogsByRating(float min, float max)`
Linear scan. Returns logs in rating range `[min, max]`.

**Complexity:** O(n)

**Example:**
```cpp
vector<FilmLog> topRated = db.getLogsByRating(4.5, 5.0);
```

#### `vector<FilmLog> getUserTopLogs(const char* username, int k)`
Hash index lookup + B-Tree fetch. Returns user's top `k` logs sorted by rating (descending).

**Complexity:** O(m log m) where m = user's total logs

**Example:**
```cpp
vector<FilmLog> top10 = db.getUserTopLogs("alice", 10);
```

### Utility

#### `void save()`
No-op. Data is persisted automatically on B-Tree writes.

#### `void rebuildHashIndex()`
Scans entire B-Tree and rebuilds in-memory hash map. Called on startup.

## CLI Commands

```
add <user> <movie> <rating>       # Add new log
get <id>                          # Retrieve log by ID
top <user>                        # Get user's top 10 rated films
update <id> <movie> <rating>      # Update existing log
delete <id>                       # Delete log
search <substring>                # Find movies by title
rating <min> <max>                # Get logs in rating range
deleteall                         # Wipe entire database (confirmation required)
save                              # Manual save (auto-saved anyway)
exit                              # Quit
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Add log | O(log n) | B-Tree insert + hash insert |
| Get by ID | O(log n) | B-Tree search |
| Delete log | O(log n) | B-Tree delete + hash cleanup |
| Update log | O(log n) | B-Tree update |
| Search title | O(n) | Full tree scan |
| Rating range | O(n) | Full tree scan |
| User top logs | O(m log m) | Hash lookup + sort |
| Delete all | O(n) | File deletion + reinit |

**Legend:**
- n = total logs in database
- m = logs for specific user

## Memory Model

**Stack Allocations:**
- BTree file handles (minimal)
- Function-local FilmLog structs

**Heap Allocations:**
- B-Tree nodes (disk-backed, paged on demand)
- HashMap buckets array (resized dynamically)
- HashMap chain nodes (one per unique key)
- String copies for hash keys (ownership transfer)

**No STL Containers Used For:**
- Hash map implementation (custom `HashMap`)
- B-Tree node storage (raw byte arrays)

**STL Allowed For:**
- `std::vector` (return values only)
- `std::string` (filenames, parsing)
- `std::fstream` (file I/O)

## Constraints & Design Decisions

1. **No `std::map` or `std::unordered_map`**: Custom hash map with manual memory management ensures full control over allocation strategy.

2. **Fixed-size FilmLog fields**: Predictable serialization. No variable-length string parsing.

3. **Hash index not persisted**: Rebuilt on startup from B-Tree. Avoids sync complexity.

4. **Load factor 0.7**: Balance between memory usage and collision probability.

5. **Chaining over open addressing**: Simpler deletion logic. Better cache locality on rehash.

6. **DJB2 hash**: Fast, well-distributed for string keys. Minimal collision rate.

7. **B-Tree order 5**: Fits multiple keys per node without excessive splits. Good balance for disk I/O.

## Compilation

Compile from the `backend/` directory:

```bash
g++ -o cinelog src/main.cpp -I./include -std=c++11
```

Run:
```bash
./cinelog
```

## Future Optimizations

- **Persistent hash index**: Serialize to disk, avoid rebuild on restart
- **Range queries on rating**: Secondary B-Tree index on rating field
- **LRU cache**: Cache frequently accessed nodes in memory
- **Batch inserts**: Buffer writes, flush periodically
- **Concurrent access**: Read-write locks for multi-user scenarios
