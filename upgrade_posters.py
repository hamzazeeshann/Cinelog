import csv
import requests
import time
import sys

API_KEY = 'b3ed21b7e1c2a8df61c48c8db1065831'
TMDB_SEARCH_URL = 'https://api.themoviedb.org/3/search/movie'
TMDB_IMAGE_BASE = 'https://image.tmdb.org/t/p/w500'
TMDB_BACKDROP_BASE = 'https://image.tmdb.org/t/p/original'

print("=" * 60)
print("TMDB Poster & Backdrop Upgrader")
print("=" * 60)
print()

# Read CSV
input_csv = 'Some CSV/imdb_top_1000.csv'
output_csv = 'Some CSV/imdb_top_1000_updated.csv'

films = []
with open(input_csv, 'r', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    films = list(reader)

print(f"Loaded {len(films)} films from CSV")
print("Starting TMDB lookup (this will take a while)...")
print()

updated_count = 0
failed_count = 0

for i, film in enumerate(films):
    title = film['Series_Title']
    year = film['Released_Year'][:4] if film['Released_Year'] else ''
    
    # Search TMDB
    try:
        params = {
            'api_key': API_KEY,
            'query': title,
            'year': year if year.isdigit() else None
        }
        
        response = requests.get(TMDB_SEARCH_URL, params=params)
        
        if response.status_code == 200:
            data = response.json()
            
            if data['results']:
                result = data['results'][0]  # Take first match
                
                # Get poster and backdrop
                if result.get('poster_path'):
                    film['Poster_Link'] = TMDB_IMAGE_BASE + result['poster_path']
                
                if result.get('backdrop_path'):
                    film['Backdrop_Link'] = TMDB_BACKDROP_BASE + result['backdrop_path']
                
                updated_count += 1
                print(f"✓ [{i+1}/{len(films)}] {title} ({year})")
            else:
                failed_count += 1
                print(f"✗ [{i+1}/{len(films)}] {title} ({year}) - No TMDB match")
        else:
            failed_count += 1
            print(f"✗ [{i+1}/{len(films)}] {title} ({year}) - API error")
        
        # Rate limiting - TMDB allows 40 requests per 10 seconds
        time.sleep(0.26)
        
    except Exception as e:
        failed_count += 1
        print(f"✗ [{i+1}/{len(films)}] {title} ({year}) - Error: {str(e)}")
        time.sleep(0.5)

# Add Backdrop_Link column if it doesn't exist
fieldnames = list(films[0].keys())
if 'Backdrop_Link' not in fieldnames:
    fieldnames.append('Backdrop_Link')

# Write updated CSV
with open(output_csv, 'w', newline='', encoding='utf-8') as f:
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(films)

print()
print("=" * 60)
print(f"Complete!")
print(f"  Updated: {updated_count}")
print(f"  Failed: {failed_count}")
print(f"  Output: {output_csv}")
print("=" * 60)
