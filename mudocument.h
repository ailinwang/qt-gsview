#ifndef MUDOCUMENT_H
#define MUDOCUMENT_H

#include "status.h"
#include "muctx.h"

typedef char Byte;

class muDocument
{
public:
    muDocument();
    ~muDocument();

    bool Initialize();
    void CleanUp();

    bool OpenFile(const std::string fileName);
    bool ProcessFile();

private:
    muctx *mu_ctx;
};

#endif // MUDOCUMENT_H
