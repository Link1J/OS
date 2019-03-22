#include <cstdint>
#include "Color.hpp"

namespace Screen
{
	void Init(uint32_t* videoBuffer, int width, int height, bool invertColors);

	void SetPixel(Color color, int x, int y);
	void Clear   (Color color              );
	void ClearRow(Color color, int row     );
	void CopyRow (int from   , int to      );
	
	int Width ();
	int Height();
	
};