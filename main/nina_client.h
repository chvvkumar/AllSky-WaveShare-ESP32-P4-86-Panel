#pragma once

#include "ui/nina_dashboard.h"

// Fetch data from a NINA instance
// base_url: e.g., "http://astromele2.lan:1888/v2/api/"
// data: pointer to NinaData struct to populate
void nina_client_get_data(const char *base_url, NinaData *data);
