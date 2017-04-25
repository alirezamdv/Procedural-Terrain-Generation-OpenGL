//
// Exported functions

void drawPerlin(FrameBuffer& noiseBuffer);

void updatePerlinOffsetTop();
void updatePerlinOffsetRight();
void updatePerlinOffsetBottom();
void updatePerlinOffsetLeft();

// The rest of this file is private implementation
// Do not access

namespace gridParams {
  using gridCoord_t = array<int, 2>;
  enum CoordSelector { I = 0, J = 1 };

  template <class T>
  array<T, 2> makeIJVector(T iValue, T jValue) {
    array<T, 2> c; c[I] = iValue; c[J] = jValue;
    return c;
  }
  gridCoord_t makeCoords(int i, int j) {
    return makeIJVector(i, j);
  }

  const gridCoord_t coordMax = makeCoords(20, 20);
  gridCoord_t bottomLeftTile = makeCoords(0, 0);

  int coordInBottomLeftReferential(CoordSelector ij, int coordInGridReferential) {
    return (coordInGridReferential + coordMax[ij] - bottomLeftTile[ij]) % coordMax[ij];
  }
  int coordInGridReferential(CoordSelector ij, int coordInBottomLeftReferential) {
    return (coordInBottomLeftReferential + bottomLeftTile[ij]) % coordMax[ij];
  }
  void updateCoord(CoordSelector ij, int direction, gridCoord_t& coords) {
    coords[ij] = (coords[ij] + coordMax[ij] + direction) % coordMax[ij];
  }
  void increaseCoord(CoordSelector ij, gridCoord_t& coords) {
    updateCoord(ij, +1, coords);
  }
  void decreaseCoord(CoordSelector ij, gridCoord_t& coords) {
    updateCoord(ij, -1, coords);
  }
}

namespace textureParams {
  const int X = 0, Y = 1;
  const vec2 textureScale = {1.0f / gridParams::coordMax[gridParams::J],
                             1.0f / gridParams::coordMax[gridParams::I]};
  const float xOriginalWidth = 2.0f;
  const float yOriginalHeight = 2.0f;
  const float xWidth = xOriginalWidth * textureScale[X];
  const float yHeight = yOriginalHeight * textureScale[Y];
  const vec3 toTextureBottomLeft = vec3(-1 + xWidth / 2.0f, -1 + yHeight / 2.0, 0);

  vec3 translationForGridCoords(const gridParams::gridCoord_t& gridCoords) {
    return toTextureBottomLeft + vec3{gridCoords[gridParams::J] * xWidth, gridCoords[gridParams::I] * yHeight, 0};
  }
}

namespace noiseParams {
  const array<vec2, 2> offsetGainPerGridBand = gridParams::makeIJVector(vec2(0, 1), vec2(1, 0));
  vec2 bottomLeftTileOffset = {0, 0};
  float noiseBufferHeight = 1024;
  float noiseBufferWidth = 1024;

  vec2 noiseOffsetForTile(gridParams::gridCoord_t tile) {
    return bottomLeftTileOffset
      + offsetGainPerGridBand[gridParams::I] * static_cast<float>(tile[gridParams::I])
      + offsetGainPerGridBand[gridParams::J] * static_cast<float>(tile[gridParams::J]);
  }
}

namespace gridParams {
    template <class Perlin>
    void redrawGridBand(gridParams::CoordSelector S, int sCoordValue, Perlin& perlin) {
      using namespace textureParams;
      using namespace noiseParams;
      CoordSelector T = (S == I) ? J : I;
      gridCoord_t gridCoord;
      gridCoord_t tileCoord;
      gridCoord[S] = sCoordValue;
      tileCoord[S] = coordInBottomLeftReferential(S, gridCoord[S]);
      for (tileCoord[T] = 0; tileCoord[T] < coordMax[T]; ++tileCoord[T]) {
        gridCoord[T] = coordInGridReferential(T, tileCoord[T]);
        perlin.Draw(noiseOffsetForTile(tileCoord),
                    textureScale,
                    translationForGridCoords(gridCoord));
      }
    }
    template <class Perlin>
    void redrawGridBand2(gridParams::CoordSelector S, int sCoordValue, Perlin& perlin) {
      using namespace textureParams;
      using namespace noiseParams;
      CoordSelector T = (S == I) ? J : I;
      gridCoord_t gridCoord;
      gridCoord_t tileCoord;
      gridCoord[S] = sCoordValue;
      tileCoord[S] = coordInBottomLeftReferential(S, gridCoord[S]);
      for (tileCoord[T] = 0; tileCoord[T] < coordMax[T]; ++tileCoord[T]) {
        gridCoord[T] = coordInGridReferential(T, tileCoord[T]);

        perlin.updateScale(vec2(1.0/coordMax[J], 1.0/coordMax[I]));

        perlin.updateOffset(vec2(gridCoord[J], gridCoord[I]));

        perlin.Draw({},
                    textureScale,
                    translationForGridCoords(gridCoord));
      }
    }

    void updatePerlinOffset(gridParams::CoordSelector bandSelector, int direction) {
      using namespace gridParams;
      using namespace textureParams;
      using namespace noiseParams;

      gridCoord_t oldBottomLeftTile = bottomLeftTile;
      updateCoord(bandSelector, direction, bottomLeftTile);
      bottomLeftTileOffset += static_cast<float>(direction) * offsetGainPerGridBand[bandSelector];
      fractionalView.zoomOffset = vec2(bottomLeftTileOffset.x/coordMax[J], bottomLeftTileOffset.y/coordMax[I]);

      // draw perlin noise
      noiseBuffer.Bind();
      if (direction < 0) {
        redrawGridBand(bandSelector, bottomLeftTile[bandSelector], perlin);
      } else {
        redrawGridBand(bandSelector, oldBottomLeftTile[bandSelector], perlin);
      }
      noiseBuffer.Unbind();

      gridCoord_t oldOldBottomLeftTile = oldBottomLeftTile;
      updateCoord(bandSelector, -direction, oldOldBottomLeftTile);

      // draw normapMap (both obsolete band and previous band to smooth the map)
      normalBuffer.Bind();
      redrawGridBand2(bandSelector, oldBottomLeftTile[bandSelector], normalMap);
      if (direction < 0) {
        redrawGridBand2(bandSelector, bottomLeftTile[bandSelector], normalMap);
      } else {
        redrawGridBand2(bandSelector, oldOldBottomLeftTile[bandSelector], normalMap);
      }
      normalBuffer.Unbind();
    }
}

 void drawPerlin(FrameBuffer& noiseBuffer) {
   noiseBuffer.Bind();
   for (int jCol = 0; jCol < gridParams::coordMax[gridParams::J]; ++jCol) {
     gridParams::redrawGridBand(gridParams::J, jCol, perlin);
   }
   noiseBuffer.Unbind();
 }

 void updatePerlinOffsetLeft() {
   gridParams::updatePerlinOffset(gridParams::J, -1);
 }

 void updatePerlinOffsetRight() {
   gridParams::updatePerlinOffset(gridParams::J, +1);
 }

 void updatePerlinOffsetBottom() {
   gridParams::updatePerlinOffset(gridParams::I, -1);
 }

 void updatePerlinOffsetTop() {
   gridParams::updatePerlinOffset(gridParams::I, +1);
 }
