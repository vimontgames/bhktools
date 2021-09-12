void Map::render(RenderWindow & _window)
{
    extern u32 g_screenWidth;
    extern u32 g_screenHeight;

    // TODO: per-map view settings?
    extern Vector2f g_cameraOffset;
    extern float g_cameraZoom;

    if (renderTexture.getSize().x != g_screenWidth || renderTexture.getSize().y != g_screenHeight)
        renderTexture.create(g_screenWidth, g_screenHeight);

    renderTexture.clear();

    sf::View view;
             view.setCenter(sf::Vector2f(float(g_screenWidth)*0.5f, (float)(g_screenHeight)*0.5f));
             view.setSize(sf::Vector2f(float(g_screenWidth), (float)g_screenHeight));
             view.zoom(g_cameraZoom); // zeng
             view.move(g_cameraOffset);

    renderTexture.setView(view);

    for (u32 i = 0; i < MapBitmap::Count; ++i)
    {
        auto & bitmap = bitmaps[i];
        if (bitmap.visible)
        {
            if (bitmap.drawQuad)
            {
                auto & texture = bitmap.texture;
                texture.setRepeated(true);

                auto & sprite = bitmap.sprite;

                sf::Shader * shader = ShaderManager::get(bitmap.quadshader);

                RenderStates rs;
                rs.shader = shader;
                rs.blendMode = bitmap.quadblend;

                if (shader)
                {
                    shader->setUniform("texSize", (Vector2f)texture.getSize());
                    shader->setUniform("screenSize", Vector2f(float(g_screenWidth), float(g_screenHeight)));

                    int passFlags = 0;

                    switch (territoryBackground)
                    {
                    default:
                    case TerritoryBackground::None:
                        break;

                    case TerritoryBackground::Tile:
                        passFlags = PASS_TYPE_TILE;
                        break;

                    case TerritoryBackground::Territory:
                        passFlags = PASS_TYPE_TERRITORY;
                        break;

                    case TerritoryBackground::Biome:
                        passFlags = PASS_TYPE_BIOME;
                        break;

                    case TerritoryBackground::Landmarks:
                        passFlags = PASS_TYPE_LANDMARK;
                        break;

                    case TerritoryBackground::Wonders:
                        passFlags = PASS_TYPE_WONDER;
                        break;
                    }

                    if (useHexUVs)
                        passFlags |= PASS_FLAG_HEXES;

                    if (showTerritoriesBorders)
                        passFlags |= PASS_FLAG_BORDERS;

                    shader->setUniform("passFlags", passFlags);
                }

                renderTexture.draw(sprite, rs);
            }

            if (bitmap.drawSprites)
            {
                sf::Shader * shader = ShaderManager::get(bitmap.spriteshader);

                RenderStates rs;
                rs.shader = shader;
                rs.blendMode = bitmap.spriteblend;

                for (u32 j = 0; j < bitmap.sprites.size(); ++j)
                {
                    Sprite & sprite = bitmap.sprites[j];

                    if (shader)
                    {
                        shader->setUniform("texSize", (Vector2f)sprite.getTexture()->getSize());
                        shader->setUniform("screenSize", Vector2f(float(g_screenWidth), float(g_screenHeight)));

                        auto color = Glsl::Vec4(sprite.getColor());
                        shader->setUniform("color", color);
                    }

                    renderTexture.draw(sprite, rs);
                }
            }
        }
    }
    renderTexture.display();

    const Texture & tex = renderTexture.getTexture();
    sf::Sprite sprite(tex);
    _window.draw(sprite);
}