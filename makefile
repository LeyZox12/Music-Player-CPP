CXXFLAGS = -std=c++23 -g -I"../../../raylib/include" -L"../../../raylib/lib" 

final : main.cpp
	g++.exe $(CXXFLAGS) main.cpp -o music.exe -lraylib -lglfw3 -lopengl32 -lgdi32 -lwinmm -lole32 -luuid -lcomdlg32 -lshell32 -I../../../taglib-2.1.1/include -L../../../taglib-2.1.1/lib -l"tag.dll"
