#pragma once

static std::string ConvertSlimToFullUUID(const std::string& uuid)
{
    if(uuid.length() != 32)
    {
        MCLog::warn("string \"{}\" is either too long or too short to be a uuid");
        return "";
    }
    
    std::string id;
    id.append(uuid.data(), uuid.data() + 8);
    id.append("-");
    id.append(uuid.data() + 8, uuid.data() + 8 + 4);
    id.append("-");
    id.append(uuid.data() + 8 + 4, uuid.data() + 8 + 4 + 4);
    id.append("-");
    id.append(uuid.data() + 8 + 4 + 4, uuid.data() + 8 + 4 + 4 + 4);
    id.append("-");
    id.append(uuid.data() + 8 + 4 + 4 + 4, uuid.data() + 8 + 4 + 4 + 4 + 12);

    return id;
}