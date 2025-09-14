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
#include <mpegfile.h>
#include <attachedpictureframe.h>
#include <id3v2tag.h>

using namespace std;
//TODO make albums iso + put them in the right place + placeholder if no cover
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
    vector<Track> tracks;
    string name;
    Rectangle rect;
    Texture2D cover;
    bool hasImage = false;
};

const vec2 rectSize = vec2(200.f, 200.f);
const float coverSize = 100.f;

struct Drawer
{
    void draw(vec2 gridPos, Texture2D& drawer, Texture2D& drawerPart, bool opened)
    {
        float x = gridPos.x * rectSize.x * 0.65f;
        float y = gridPos.y * rectSize.y * 0.325f;
        float scale = rectSize.x / drawer.width;
        float targetOffset = opened ? -5.f : 0.f;
        offset += (targetOffset - offset) / 5.f;
        for(int i = 10; i > 0; i--)
        {
            vec2 pos = vec2(x + rectSize.x * 0.2f * (i+offset), (x-rectSize.x * 0.2f * (i+offset))/1.75f + y);
            DrawTextureEx(drawer, pos, 0, scale, WHITE);
            pos = vec2(x + rectSize.x * 0.2f * i, (x-rectSize.x * 0.2f * i)/1.75f + y);
            if(gridPos.y == 0)
                DrawTextureEx(drawerPart, pos, 0, scale, WHITE);

        } 
        x = gridPos.x * rectSize.x * 0.65f;
        DrawTextureEx(drawer, vec2(x + rectSize.x * offset * 0.2f, (x-rectSize.x * 0.2f * offset)/1.75f + y), 0, scale, WHITE);
        if(gridPos.y == 0)
            DrawTextureEx(drawerPart, vec2(x, x/1.75f), 0, scale, WHITE);
    }
    float offset = 0.f;
    string artist = "";
    vector<Album> albums;
};

vector<Drawer> drawers;
vec2 pos = vec2(0, 0);

const int PROGRESS_PRECISION = 20;
const float MOVE_SPEED = 3.f;

int getDrawerIndex();

int main()
{
    InitWindow(512, 512, "epic music player");
    SetTargetFPS(60);
    string path = "../../../../Music";
    path = "C:/Users/solat/Music";
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
                int songIndex = 0;
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

                        if(songIndex++ == 0)
                        {
                            TagLib::File* file = f.file();
                            TagLib::MPEG::File* f2((TagLib::MPEG::File*)file);
                            TagLib::ID3v2::Tag *m_tag = f2->ID3v2Tag(true);
                            TagLib::ID3v2::FrameList fl = m_tag->frameList("APIC");
                            if(!fl.isEmpty())
                            {
                                TagLib::ID3v2::AttachedPictureFrame *coverImg = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(fl.front());
                                albumObject.cover = LoadTextureFromImage(LoadImageFromMemory(".png", reinterpret_cast<const unsigned char *>(coverImg->picture().data()),coverImg->picture().size()));
                            }
                            else
                            {
                                cout << "cover not found\n";
                            }
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
        


    int openedIndex = -1;
    bool opened = false;
    float dt = 1.f / 60.f;
    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        int ratio = floor(drawers.size()/3.f);
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            int index = getDrawerIndex();
            if(index > -1 && index < drawers.size())
            {
                opened = !opened;
                openedIndex = index;
            }

        }
        if(IsKeyDown(KEY_LEFT)) pos.x += MOVE_SPEED * dt;
        else if(IsKeyDown(KEY_RIGHT)) pos.x -= MOVE_SPEED * dt;
        for(int y = 2; y >= 0; y--)
        for(int i = 0; i < ratio; i++)
        {
            bool isOpened = openedIndex == i+y*ratio && opened;
            drawers[i+y*ratio].draw(vec2(i+pos.x, y), drawerTex, drawerPartTex, isOpened);
        }
        if(opened && openedIndex > -1)
        {
            int i = 0;
            for(auto& album : drawers[openedIndex].albums)
            {
                float ratio = coverSize / (float)album.cover.width;
                DrawTextureEx(album.cover, vec2(i++ * 100, 0), 0, ratio, WHITE);
            }
        }
        EndDrawing();
    }
    return 0;
}

int getDrawerIndex()
{
    vec2 mousepos = GetMousePosition();
    int x = floor((mousepos.x-pos.x * rectSize.x * 0.65f) / 128.f);
    int y = floor((mousepos.y-mousepos.x*0.558f) / 67.f)-1;
    int index = x + y*(floor(drawers.size()/3.f));
    if(index < drawers.size()) return index;
    return -1;
}
