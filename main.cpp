#include "raylib.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cmath>
#include <filesystem>
#include <vector>
#include <string>
#include "tag.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "tvariant.h"
#include "fileref.h"

using namespace std;

typedef Vector2 vec2;

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
    void draw(vec2 gridPos, Texture2D& drawer, Texture2D& drawerPart)
    {
        float x = gridPos.x * rectSize.x * 0.65f;
        float y = gridPos.y * rectSize.y * 0.325f;
        float scale = rectSize.x / drawer.width;
        
        for(int i = 10; i > 0; i--)
        {
            vec2 pos = vec2(x + rectSize.x * 0.2f * i, (x-rectSize.x * 0.2f * i)/1.75f + y);
            DrawTextureEx(drawer, pos, 0, scale, WHITE);
            if(gridPos.y == 0)
                DrawTextureEx(drawerPart, pos, 0, scale, WHITE);

        }   
        x = gridPos.x * rectSize.x * 0.65f;
        DrawTextureEx(drawer, vec2(x, x/1.75f + y), 0, scale, WHITE);
        if(gridPos.y == 0)
            DrawTextureEx(drawerPart, vec2(x, x/1.75f), 0, scale, WHITE);
    }
    vec2 rectSize = vec2(200, 200);
    string artist = "";
    vector<Album> albums;
};

const int PROGRESS_PRECISION = 20;
const float MOVE_SPEED = 3.f;

int main()
{
    InitWindow(512, 512, "epic music player");
    SetTargetFPS(60);
    vector<Drawer> drawers;
    string path = "C:/Users/solat/Music";
    Texture2D drawerPartTex = LoadTexture("res/drawerPart.png");
    Texture2D drawerTex = LoadTexture("res/drawer.png");


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
    int a = 0;
    for(auto& artist : artists)
    {
        string progress = "";
        for(int i = 0; i < PROGRESS_PRECISION; i++)
        {
            if(i < ceil(a / (float)artists.size() * PROGRESS_PRECISION)) progress += "I";
            else progress += "_";
        }
        cout <<"progress:" <<  progress << endl;
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
        a++;
    }

    sort(drawers.begin(), drawers.end(), [](auto& d1, auto& d2){return (uint8_t)d1.artist[0] < (uint8_t)d2.artist[0];});
    for(auto& drawer: drawers)
    {
        cout << drawer.artist << "'s drawer:" << endl;
        /*for(auto& album : drawer.albums){
            cout << "   " << album.name <<"'s tracks:" <<endl;
            for(auto& track: album.tracks)
            {
                cout << "       " << track.name << endl;
            }

        }*/
    }
        
    vec2 pos = vec2(0, 0);
    vec2 openedPos = vec2(0, 0);
    bool opened = false;///TODO implement opening drawer on click
    float dt = 1.f / 60.f;
    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        int ratio = floor(drawers.size()/3.f);
        if(IsKeyDown(KEY_LEFT)) pos.x -= MOVE_SPEED * dt;
        else if(IsKeyDown(KEY_RIGHT)) pos.x += MOVE_SPEED * dt;
        for(int y = 3; y >= 0; y--)
        for(int i = 0; i < ratio; i++)
        {
            drawers[i].draw(vec2(i+pos.x, y), drawerTex, drawerPartTex);
        }
        EndDrawing();
    }
    return 0;
}
