void setupBlocks(int startX, int startY) {
    float blockWidth = 5.0f;
    float blockHeight = 2.0f;
    float startBlockX = startX + 2.0f;
    float startBlockY = startY + 3.0f;
    float spacing = 1.0f;

    int rows = 5;
    int cols = static_cast<int>((gameArea->getWidth() - 4 + spacing) / (blockWidth + spacing));

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float x = startBlockX + col * (blockWidth + spacing);
            float y = startBlockY + row * (blockHeight + spacing);

            // Assign different hit points and scores based on row
            int hitPoints = std::min(3, rows - row);
            int blockScore = hitPoints * 50;
            int colorPair = 3 + (3 - hitPoints); // Different colors based on hit points
            
            blocks.push_back(new Block(x, y, blockWidth, blockHeight, hitPoints, blockScore, colorPair));
        }
    }
}
