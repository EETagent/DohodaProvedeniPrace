
#include <qrencode.h> // libqrencode, tvorba QR kódu
#include <png.h>      // libpng, tvorba PNG obrázku

QRcode* qrEncodeString(const char *string);

int qrWritePNG(const QRcode *qrcode, const char *outfile);
