def space_count( grid_in_tile, grid_width, grid_height, intsize ):
    tile_num = ( grid_width / grid_in_tile ) * ( grid_height / grid_in_tile )
    bit = tile_num * ( 
        ( 8 + 8 + (30 * 8) + intsize + intsize + ( 2 * ( grid_in_tile * intsize ) ) )
    )
    print ( bit )
    print ( bit / 8 )
    print ( bit / 8 / 1024 )

space_count ( 20, 1900, 300, 16 )
space_count ( 20, 1900, 300, 32 )
