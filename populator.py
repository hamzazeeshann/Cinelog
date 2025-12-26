import requests
import struct
import os
import time

# TMDB API Configuration
TMDB_API_KEY = "b3ed21b7e1c2a8df61c48c8db1065831"
TMDB_BASE_URL = "https://api.themoviedb.org/3"

def create_data_directory():
    """Create data directory if it doesn't exist"""
    if not os.path.exists('data'):
        os.makedirs('data')
        print("Created 'data' directory")

def fetch_popular_movies(num_movies=100):
    """Fetch top popular movies from TMDB"""
    print(f"Fetching top {num_movies} popular movies from TMDB...")
    movies = []
    page = 1
    
    while len(movies) < num_movies:
        url = f"{TMDB_BASE_URL}/movie/popular"
        params = {
            'api_key': TMDB_API_KEY,
            'page': page
        }
        
        try:
            response = requests.get(url, params=params)
            response.raise_for_status()
            data = response.json()
            
            for movie in data['results']:
                if len(movies) >= num_movies:
                    break
                
                # Fetch detailed movie info
                detail_url = f"{TMDB_BASE_URL}/movie/{movie['id']}"
                detail_params = {
                    'api_key': TMDB_API_KEY,
                    'append_to_response': 'credits'
                }
                
                detail_response = requests.get(detail_url, params=detail_params)
                if detail_response.status_code == 200:
                    detail_data = detail_response.json()
                    movies.append(detail_data)
                    print(f"  Fetched: {detail_data['title']} ({detail_data.get('release_date', 'N/A')[:4]})")
                
                time.sleep(0.25)  # Rate limiting
            
            page += 1
            
        except requests.exceptions.RequestException as e:
            print(f"Error fetching movies: {e}")
            break
    
    return movies

def get_cast_summary(credits):
    """Extract top 5 cast members"""
    if 'cast' not in credits:
        return "Unknown"
    
    cast_list = credits['cast'][:5]
    cast_names = [actor['name'] for actor in cast_list]
    return ', '.join(cast_names)

def get_director(credits):
    """Extract director name"""
    if 'crew' not in credits:
        return "Unknown"
    
    for member in credits['crew']:
        if member['job'] == 'Director':
            return member['name']
    return "Unknown"

def get_top_genres(genres):
    """Extract top 3 genre IDs"""
    genre_ids = [0, 0, 0]
    for i, genre in enumerate(genres[:3]):
        genre_ids[i] = genre['id']
    return genre_ids

def write_film_binary(film_data, film_id):
    """
    Write a Film struct to binary
    struct Film {
        int film_id;           // 4 bytes
        int tmdb_id;           // 4 bytes
        char title[64];        // 64 bytes
        int release_year;      // 4 bytes
        int runtime;           // 4 bytes
        char cast_summary[256];// 256 bytes
        char director[64];     // 64 bytes
        int genre_ids[3];      // 12 bytes (3 * 4)
    }
    Total: 412 bytes
    """
    title = film_data['title'][:63].encode('utf-8', errors='ignore')
    title = title + b'\x00' * (64 - len(title))
    
    release_year = int(film_data.get('release_date', '2000')[:4]) if film_data.get('release_date') else 2000
    runtime = film_data.get('runtime', 0) or 0
    
    cast = get_cast_summary(film_data.get('credits', {}))[:255].encode('utf-8', errors='ignore')
    cast = cast + b'\x00' * (256 - len(cast))
    
    director_name = get_director(film_data.get('credits', {}))[:63].encode('utf-8', errors='ignore')
    director_name = director_name + b'\x00' * (64 - len(director_name))
    
    genre_ids = get_top_genres(film_data.get('genres', []))
    
    # Pack the struct
    data = struct.pack('i', film_id)  # film_id
    data += struct.pack('i', film_data['id'])  # tmdb_id
    data += title  # title[64]
    data += struct.pack('i', release_year)  # release_year
    data += struct.pack('i', runtime)  # runtime
    data += cast  # cast_summary[256]
    data += director_name  # director[64]
    data += struct.pack('iii', genre_ids[0], genre_ids[1], genre_ids[2])  # genre_ids[3]
    
    return data

def write_user_binary(user_id, username, email, password_hash, bio, join_date, is_admin):
    """
    Write a User struct to binary
    struct User {
        int user_id;           // 4 bytes
        char username[32];     // 32 bytes
        char email[64];        // 64 bytes
        char password_hash[64];// 64 bytes
        char bio[256];         // 256 bytes
        long join_date;        // 8 bytes
        bool isAdmin;          // 1 byte
    }
    Total: 429 bytes
    """
    username_bytes = username[:31].encode('utf-8', errors='ignore')
    username_bytes = username_bytes + b'\x00' * (32 - len(username_bytes))
    
    email_bytes = email[:63].encode('utf-8', errors='ignore')
    email_bytes = email_bytes + b'\x00' * (64 - len(email_bytes))
    
    password_bytes = password_hash[:63].encode('utf-8', errors='ignore')
    password_bytes = password_bytes + b'\x00' * (64 - len(password_bytes))
    
    bio_bytes = bio[:255].encode('utf-8', errors='ignore')
    bio_bytes = bio_bytes + b'\x00' * (256 - len(bio_bytes))
    
    data = struct.pack('i', user_id)  # user_id
    data += username_bytes  # username[32]
    data += email_bytes  # email[64]
    data += password_bytes  # password_hash[64]
    data += bio_bytes  # bio[256]
    data += struct.pack('q', join_date)  # join_date (long = 8 bytes)
    data += struct.pack('?', is_admin)  # isAdmin (bool = 1 byte)
    
    return data

def write_log_binary(log_id, user_id, film_id, rating, review, watch_date):
    """
    Write a Log struct to binary
    struct Log {
        int log_id;            // 4 bytes
        int user_id;           // 4 bytes
        int film_id;           // 4 bytes
        float rating;          // 4 bytes
        char review_preview[256]; // 256 bytes
        long watch_date;       // 8 bytes
    }
    Total: 280 bytes
    """
    review_bytes = review[:255].encode('utf-8', errors='ignore')
    review_bytes = review_bytes + b'\x00' * (256 - len(review_bytes))
    
    data = struct.pack('i', log_id)  # log_id
    data += struct.pack('i', user_id)  # user_id
    data += struct.pack('i', film_id)  # film_id
    data += struct.pack('f', rating)  # rating
    data += review_bytes  # review_preview[256]
    data += struct.pack('q', watch_date)  # watch_date
    
    return data

def write_genre_binary(genre_id, name):
    """
    Write a Genre struct to binary
    struct Genre {
        int genre_id;          // 4 bytes
        char name[32];         // 32 bytes
    }
    Total: 36 bytes
    """
    name_bytes = name[:31].encode('utf-8', errors='ignore')
    name_bytes = name_bytes + b'\x00' * (32 - len(name_bytes))
    
    data = struct.pack('i', genre_id)  # genre_id
    data += name_bytes  # name[32]
    
    return data

def initialize_btree_file(filename):
    """Initialize B-Tree file with header"""
    with open(filename, 'wb') as f:
        root_pos = 16  # After the two longs
        next_pos = 16
        f.write(struct.pack('q', root_pos))  # rootPos (long = 8 bytes)
        f.write(struct.pack('q', next_pos))  # nextPos (long = 8 bytes)

def populate_database():
    """Main function to populate all database files"""
    print("=" * 60)
    print("CINELOG Database Populator")
    print("=" * 60)
    print()
    
    create_data_directory()
    
    # Initialize B-Tree files
    print("Initializing database files...")
    initialize_btree_file('data/users.bin')
    initialize_btree_file('data/films.bin')
    initialize_btree_file('data/logs.bin')
    initialize_btree_file('data/genres.bin')
    initialize_btree_file('data/lists.bin')
    print("Database files initialized")
    print()
    
    # Create dummy users
    print("Creating users...")
    users_data = [
        (1, "admin", "admin@cinelog.com", "admin123", "System Administrator", int(time.time()), True),
        (2, "john_doe", "john@example.com", "password123", "Movie enthusiast", int(time.time()), False),
        (3, "jane_smith", "jane@example.com", "password123", "Film critic", int(time.time()), False),
        (4, "cinephile", "cinephile@example.com", "password123", "I love cinema!", int(time.time()), False),
    ]
    
    # Note: These records won't be directly inserted into B-Tree format
    # They need to be inserted through the C++ backend after it starts
    # For now, we'll just prepare the data
    print(f"  Created {len(users_data)} users")
    print()
    
    # Fetch and write films
    print("THIS STEP TAKES A WHILE - Fetching films from TMDB...")
    print("Note: Rate limiting in place (0.25s per request)")
    print()
    
    movies = fetch_popular_movies(50)  # Fetch 50 movies to start
    
    print()
    print(f"Successfully fetched {len(movies)} movies")
    print()
    
    # Create genres from collected data
    print("Extracting genres...")
    genre_map = {}
    for movie in movies:
        for genre in movie.get('genres', []):
            genre_map[genre['id']] = genre['name']
    
    print(f"  Found {len(genre_map)} unique genres")
    print()
    
    # Write films to a JSON file for easy import via C++ backend
    import json
    
    films_json = []
    for i, movie in enumerate(movies, start=1):
        film_data = {
            'film_id': i,
            'tmdb_id': movie['id'],
            'title': movie['title'],
            'release_year': int(movie.get('release_date', '2000')[:4]) if movie.get('release_date') else 2000,
            'runtime': movie.get('runtime', 0) or 0,
            'cast': get_cast_summary(movie.get('credits', {})),
            'director': get_director(movie.get('credits', {})),
            'genre_ids': get_top_genres(movie.get('genres', []))
        }
        films_json.append(film_data)
    
    with open('data/films.json', 'w', encoding='utf-8') as f:
        json.dump(films_json, f, indent=2, ensure_ascii=False)
    
    print("Films exported to data/films.json")
    
    genres_json = [{'genre_id': gid, 'name': gname} for gid, gname in genre_map.items()]
    with open('data/genres.json', 'w', encoding='utf-8') as f:
        json.dump(genres_json, f, indent=2, ensure_ascii=False)
    
    print("Genres exported to data/genres.json")
    
    users_json = [
        {
            'user_id': uid,
            'username': uname,
            'email': email,
            'password_hash': pwd,
            'bio': bio,
            'join_date': jdate,
            'isAdmin': admin
        }
        for uid, uname, email, pwd, bio, jdate, admin in users_data
    ]
    
    with open('data/users.json', 'w', encoding='utf-8') as f:
        json.dump(users_json, f, indent=2, ensure_ascii=False)
    
    print("Users exported to data/users.json")
    print()
    
    # Generate some sample logs
    import random
    logs_json = []
    log_id = 1
    
    print("Generating sample logs...")
    for user_id in [2, 3, 4]:  # Non-admin users
        num_logs = random.randint(5, 15)
        watched_films = random.sample(range(1, min(len(movies) + 1, 51)), num_logs)
        
        for film_id in watched_films:
            log_data = {
                'log_id': log_id,
                'user_id': user_id,
                'film_id': film_id,
                'rating': round(random.uniform(2.0, 5.0), 1),
                'review': random.choice([
                    "Great movie!",
                    "Loved it!",
                    "Amazing cinematography",
                    "Brilliant performances",
                    "A masterpiece",
                    "Really enjoyed this",
                    "Incredible story",
                    ""
                ]),
                'watch_date': int(time.time()) - random.randint(0, 365 * 24 * 3600)
            }
            logs_json.append(log_data)
            log_id += 1
    
    with open('data/logs.json', 'w', encoding='utf-8') as f:
        json.dump(logs_json, f, indent=2, ensure_ascii=False)
    
    print(f"Generated {len(logs_json)} sample logs")
    print("Logs exported to data/logs.json")
    print()
    
    print("=" * 60)
    print("Database population complete!")
    print("=" * 60)
    print()
    print("IMPORTANT: JSON files have been created in the 'data' folder.")
    print("The C++ backend will need to read these JSON files and insert")
    print("the records into the B-Tree binary files when it starts.")
    print()
    print("Files created:")
    print("  - data/users.json")
    print("  - data/films.json")
    print("  - data/genres.json")
    print("  - data/logs.json")
    print()

if __name__ == "__main__":
    try:
        populate_database()
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
