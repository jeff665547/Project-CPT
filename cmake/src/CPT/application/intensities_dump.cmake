screw_extend_template()
target_link_libraries(${__screw_target} PUBLIC 
    Affy::affy 
    ChipImgProc::ChipImgProc-hough_transform
    Boost::serialization
    Boost::iostreams
)
