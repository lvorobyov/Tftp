//
// Created by Lev on 21.02.2019.
//

#include "detector.h"

const char *tftp::detector::detect(uint8_t bytes) {
    // Detect is there letters or special characters
    // Detect Unicode
    // Count unprintable characters
    // If is text then Match regex for xml? etc.
    // Else match bytes of data or use special algorithms
    return "text/plain";
}
