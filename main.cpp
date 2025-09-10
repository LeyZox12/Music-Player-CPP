#include "raylib.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdio>

#include <filesystem>
#include <vector>
#include <string>
#include "tag.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "tvariant.h"
#include "fileref.h"

using namespace std;

struct Track
{
    Track(string path, string name):path(path), name(name){}
    string path;
    string name;
};

struct Album
{
    Album(){};
    Album(vector<Track> tracks, Texture2D icon, bool hasImage)
    :tracks(tracks), icon(icon), hasImage(hasImage) {}
    vector<Track> tracks;
    string name;
    Texture2D icon;
    Rectangle rect;
    bool hasImage = false;
};

struct Drawer
{
    void draw()
    {
        
    }
    string artist = "";
    vector<Album> albums;
};

int main()
{
    InitWindow(512, 512, "epic music player");
    vector<Drawer> drawers;
    string path = "C:/Users/solat/Music";
    Texture2D drawerPartTex = LoadTexture("res/drawerPart.png");
    Image img = LoadImage("res/drawer.png");
    Texture2D drawerTex = LoadTextureFromImage(img);
    UnloadImage(img);


    //---------------------------FINDS ALL ARTISTS----------------------------
    vector<string> artists;
    for(auto& album : filesystem::directory_iterator(path))
    {
        if(filesystem::is_directory(album))
        {
                auto it = filesystem::directory_iterator(album);
                const char* p = it->path().string().c_str();
                TagLib::FileRef f(p);
                if(!f.isNull() && f.tag())
                {
                    TagLib::Tag *tag = f.tag();
                    string str = tag->artist().to8Bit();
                    if(count(artists.begin(), artists.end(), str) == 0)
                        artists.push_back(str);
                }
        }
    }

    //---ADDS A DRAWER FOR EACH ARTIST AND ADD EACH ALBUM FROM ARTIST TO DRAWER---

    for(auto& artist : artists)
    {
        Drawer drawer;
        vector<Album> albums;
        for(auto& album : filesystem::directory_iterator(path))
        {
            //drawer.artistName = artist;
            bool shouldAdd = true;
            Album albumObject;
            vector<Track> tracks;
            if(filesystem::is_directory(album))
            {
                for(auto& song : filesystem::directory_iterator(album))
                {
                    const char* p =  song.path().string().c_str();
                    TagLib::FileRef f(p);
                    if(!f.isNull() && f.tag())
                    {
                        TagLib::Tag *tag = f.tag();
                        if(tag->artist().to8Bit() != artist)
                        {
                            tracks.clear();
                            shouldAdd = false;
                            break;
                        }
                        albumObject.name = tag->album().to8Bit();
                        tracks.emplace_back(Track(p, tag->title().to8Bit()));
                    }
                }
                if(shouldAdd && albumObject.name != "" && tracks.size() > 0)
                {
                    albumObject.tracks = tracks;
                    albums.push_back(albumObject);
                }
            }
            else
                shouldAdd = false;
        }
        drawer.albums = albums;
        drawer.artist = artist;
        drawers.push_back(drawer);
    }


    for(auto& drawer: drawers)
    {
        cout << drawer.artist << "'s drawer:" << endl;
        for(auto& album : drawer.albums){
            cout << "   " << album.name <<"'s tracks:" <<endl;
            for(auto& track: album.tracks)
            {
                cout << "       " << track.name << endl;
            }

        }
    }
        
    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }
    return 0;
}
