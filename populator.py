import csv
import json
import struct
import time
import os
import random

print("=" * 60)
print("CINELOG Database Populator (CSV Edition)")
print("=" * 60)
print()

os.makedirs('data', exist_ok=True)

def parse_runtime(runtime_str):
    if not runtime_str or runtime_str == 'nan':
        return 120
    try:
        return int(runtime_str.replace(' min', '').strip())
    except:
        return 120

def parse_year(year_str):
    try:
        return int(str(year_str).strip()[:4])
    except:
        return 2000

# Read CSV
print("Reading CSV file...")
films = []
genre_set = set()

with open('Some CSV/imdb_top_1000.csv', 'r', encoding='utf-8') as csvfile:
    reader = csv.DictReader(csvfile)
    film_id = 1
    
    for row in reader:
        genres_str = row['Genre']
        genre_list = [g.strip() for g in genres_str.split(',')[:3]]
        while len(genre_list) < 3:
            genre_list.append('Unknown')
        
        for g in genre_list:
            genre_set.add(g)
        
        cast_parts = []
        for i in range(1, 5):
            star = row.get(f'Star{i}', '').strip()
            if star:
                cast_parts.append(star)
        cast_summary = ', '.join(cast_parts) if cast_parts else 'Unknown'
        
        try:
            rating = float(row['IMDB_Rating'])
        except:
            rating = 7.0
        
        # Clean and upgrade poster URL quality
        poster_url = row['Poster_Link'].strip()
        # Remove any trailing dots/ellipsis
        if poster_url.endswith('...'):
            poster_url = poster_url[:-3]
        elif poster_url.endswith('..'):
            poster_url = poster_url[:-2]
        
        # Upgrade Amazon image quality by modifying the URL parameters
        # Replace the small size specification with larger one
        if '_V1_UX67_CR0,0,67,98_AL_' in poster_url:
            poster_url = poster_url.replace('_V1_UX67_CR0,0,67,98_AL_', '_V1_UX300_CR0,0,300,450_AL_')
        
        # Remove .jpg extension for better quality
        if poster_url.endswith('.jpg'):
            poster_url = poster_url[:-4]
        
        film_data = {
            'film_id': film_id,
            'tmdb_id': film_id * 100,
            'title': row['Series_Title'],
            'year': parse_year(row['Released_Year']),
            'runtime': parse_runtime(row['Runtime']),
            'director': row['Director'],
            'genres': genre_list,
            'poster': poster_url,
            'overview': row['Overview'][:127] if row['Overview'] else 'No overview available',
            'rating': rating,
            'cast': cast_summary
        }
        
        films.append(film_data)
        film_id += 1

print(f"Loaded {len(films)} films from CSV")

# Create genre mapping
print("Creating genre mapping...")
genre_map = {}
genre_id = 1
for genre_name in sorted(genre_set):
    genre_map[genre_name] = genre_id
    genre_id += 1

for film in films:
    genre_ids = []
    for genre_name in film['genres']:
        genre_ids.append(genre_map.get(genre_name, 0))
    film['genre_ids'] = genre_ids

print(f"Created {len(genre_map)} genres")

# Export JSON
print("Exporting JSON files...")

films_json = []
for film in films:
    films_json.append({
        'film_id': film['film_id'],
        'tmdb_id': film['tmdb_id'],
        'title': film['title'],
        'year': film['year'],
        'runtime': film['runtime'],
        'director': film['director'],
        'genre_ids': film['genre_ids'],
        'poster_path': film['poster'],
        'backdrop_path': '',
        'tagline': film['overview'],
        'vote_average': film['rating'],
        'cast_summary': film['cast']
    })

with open('data/films.json', 'w', encoding='utf-8') as f:
    json.dump(films_json, f, indent=2, ensure_ascii=False)

print(f"Exported {len(films_json)} films to data/films.json")

genres_json = [{'genre_id': gid, 'name': gname} for gname, gid in genre_map.items()]
with open('data/genres.json', 'w', encoding='utf-8') as f:
    json.dump(genres_json, f, indent=2, ensure_ascii=False)

print(f"Exported {len(genres_json)} genres to data/genres.json")

users_data = [
    (1, 'admin', 'admin@cinelog.com', 'admin123', 'System Administrator', int(time.time()), True, 1),
    (2, 'alice', 'alice@cinelog.com', 'password123', 'Film enthusiast and critic', int(time.time()), False, 2),
    (3, 'bob', 'bob@cinelog.com', 'password123', 'Movie lover', int(time.time()), False, 3),
    (4, 'charlie', 'charlie@cinelog.com', 'password123', 'Cinema fan', int(time.time()), False, 4)
]

users_json = [
    {
        'user_id': uid,
        'username': uname,
        'email': email,
        'password_hash': pwd,
        'bio': bio,
        'join_date': jdate,
        'isAdmin': admin,
        'avatar_id': avatar
    }
    for uid, uname, email, pwd, bio, jdate, admin, avatar in users_data
]

with open('data/users.json', 'w', encoding='utf-8') as f:
    json.dump(users_json, f, indent=2, ensure_ascii=False)

print(f"Exported {len(users_json)} users to data/users.json")

# Generate logs
logs_json = []
log_id = 1

print("Generating sample logs...")

reviews = [
    "Absolutely brilliant! A masterpiece.",
    "One of the best films I've ever seen.",
    "Great cinematography and acting.",
    "A must-watch for all film lovers.",
    "Powerful and emotional.",
    "Perfect blend of story and visuals.",
    "An unforgettable experience.",
    "Simply outstanding!",
    "Exceeded all expectations.",
    "A timeless classic."
]

for user_id in [2, 3, 4]:
    num_logs = random.randint(15, 30)
    watched_films = random.sample(films[:300], min(num_logs, 300))
    
    for film in watched_films:
        rating = random.choice([3.5, 4.0, 4.5, 5.0])
        review = random.choice(reviews)
        watch_date = int(time.time()) - random.randint(0, 365*24*60*60)
        
        logs_json.append({
            'log_id': log_id,
            'user_id': user_id,
            'film_id': film['film_id'],
            'rating': rating,
            'review_text': review,
            'log_date': watch_date
        })
        
        log_id += 1

with open('data/logs.json', 'w', encoding='utf-8') as f:
    json.dump(logs_json, f, indent=2, ensure_ascii=False)

print(f"Generated {len(logs_json)} sample logs")
print()
print("=" * 60)
print("Complete! Created:")
print(f"  - {len(users_json)} users")
print(f"  - {len(films_json)} films")
print(f"  - {len(genres_json)} genres")
print(f"  - {len(logs_json)} logs")
print("=" * 60)
