# VoronoiMapGenerator
VoronoiMapGenerator는 지형 하이트맵 생성기입니다.  
VoronoiMapGenerator is a terrain heightmap generator.

Fortune's algorithm을 사용하여 각각 다르게 생긴 voronoi 셀을 생성하고 이를 기반으로 지형이 만들어집니다.  
Because Fortune's algorithm is used to generate voronoi cells and generate terrain based on them, the shape of each cell is not the same.

Fortune's algorithm 구현 원본 코드는 아래 있습니다.  
Fortune's algorithm implementation used the source code at the address below.  
(https://github.com/mdally/Voronoi)

// 해당 소스코드를 개선한 포크(https://github.com/cbk0313/Voronoi)  
// Fork with performance issues resolved (https://github.com/cbk0313/Voronoi)  

// PS) 언리얼엔진용 플러그인 제작하기 전 프로토타입 프로그램입니다.  
// PS) This is a prototype program before creating a plugin for Unreal Engine.  
# Overview
강, 해안가, 강물, 평지가 구현되어 있습니다.  
lakes, coasts, rivers, flats are implemented.  
![voronoi_map_all](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/87fcbb28-c350-47b5-bbd5-e3cb7c0d0586)

또한 봉우리와 최고점 봉우리 정보를 확인할 수 있고, 수분이 구현되어 있습니다.  
You can check peak and highest peak. additionally moisture is implemented.  
![voronoi_map_opengl](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/b19122a2-093a-406a-8027-0a227254f3cc)

아래는 생성된 하이트맵 이미지와 다른 예시 사진입니다.  
Below are the generated heightmap images and other example.  
![voronoi_map_islnad](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/a390d0e4-72ac-4e6e-a85b-92a767ff0fed)
![voronoi_map_lake](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/936bfed4-a17b-438b-a5fc-ae7dd6520162)
![voronoi_map_river](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/135cf5cd-0c37-4173-85b4-14e7ff078f01)
![voronoi_map_all](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/43ec84c3-09f9-4270-b5ab-b7d51cb62e60)
![island_example1](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/610d0cba-da1e-491f-bb0b-a24a215e80ef)
![island_example2](https://github.com/cbk0313/VoronoiMapGenerator/assets/66576971/4b1edd51-d859-4da3-9258-7dedb3a22ed4)
