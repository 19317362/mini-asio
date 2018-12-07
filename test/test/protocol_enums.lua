-- This file is generated by pcode_autog-1.18.12
-- Copyright(c) Lake.Deal, ALL RIGHTS RESERVED.
--
-- Purpose: contains some constants defininations
--
--
proto = proto or {}

proto.numbers = {
    CID_SIMPLE1 = 101, 
}

PaddingMode = {
    PKCS1_PADDING = 1,
    SSLV23_PADDING = 2,
    NO_PADDING = 3,
    PKCS1_OAEP_PADDING = 4,
    X931_PADDING = 5,
    --[[EVP_PKEY_ only]]
    PKCS1_PSS_PADDING = 6,
};

function get_file_content(fileName)
    local file = io.open(fileName)
    local content = file:read('*a')
    file:close()
    return content
end