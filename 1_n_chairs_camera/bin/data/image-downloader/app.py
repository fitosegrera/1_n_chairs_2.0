from bing_image_downloader import downloader
import sys
import os

wordCounter = 0
query = ""

for word in sys.argv:
    if wordCounter > 0:
        if wordCounter == 1:
            query += word 
        else:   
            query += " " + word
    wordCounter += 1

print("QUERY:", query)

dataPath = os.path.abspath("data/image-downloader")

downloader.download(query, limit=1,  output_dir=dataPath+'/dataset', adult_filter_off=True, force_replace=False, timeout=60, verbose=True)
