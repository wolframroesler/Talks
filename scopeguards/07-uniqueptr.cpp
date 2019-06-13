void Bind::operator+=(const db::Query& query) {

    const auto meta = std::unique_ptr<MYSQL_RES,void(*)(MYSQL_RES*)>(
        mysql_stmt_result_metadata(stmt_),
        mysql_free_result
    );
    if (!meta) {
        const auto msg = "Error getting result metadata: " + std::string(mysql_stmt_error(stmt_));
        mysql_stmt_close(stmt_);
        throw std::runtime_error(msg);
    }

    const auto Size = [this,&meta](const std::string& name){
        for(unsigned i=0;i<meta->field_count;++i) {
            if (name==meta->fields[i].name) {
                return meta->fields[i].length;
            }
        }

        mysql_stmt_close(stmt_);
        throw std::runtime_error("Error in result metadata: Length of " + name + " unknown");
    };

    for(auto& v : query.vars()) {
        const auto& data = v.second.first;
        const auto  null = v.second.second;

        std::visit(overloaded {

            [&](std::string* const dst){
                if (dst || null) {
                    const auto size = Size(v.first);
                    bufs_.push_front(std::pair(std::vector<char>(size+1),dst));
                    MYSQL_BIND b    = {nullptr};
                    b.buffer_type   = MYSQL_TYPE_STRING;
                    b.buffer        = &bufs_.front().first[0];
                    b.buffer_length = size;
                    BindNullFlag(b,null);
                    binds_.push_back(b);
                }
            },

            [&](int* const dst) {
                MYSQL_BIND b    = {nullptr};
                b.buffer_type   = MYSQL_TYPE_LONG;
                b.buffer        = dst;
                BindNullFlag(b,null);
                binds_.push_back(b);
            },

            [&](double* const dst) {
                MYSQL_BIND b    = {nullptr};
                b.buffer_type   = MYSQL_TYPE_DOUBLE;
                b.buffer        = dst;
                BindNullFlag(b,null);
                binds_.push_back(b);
            },

            [&](time_t* const dst) {
                tims_.push_front(std::pair(MYSQL_TIME(),dst));
                MYSQL_BIND b    = {nullptr};
                b.buffer_type   = MYSQL_TYPE_DATETIME;
                b.buffer        = &tims_.front().first;
                b.buffer_length = sizeof(tims_.front().first);
                BindNullFlag(b,null);
                binds_.push_back(b);
            },

            [&](std::nullptr_t) {
                if (null) {
                    const auto size = Size(v.first);
                    bufs_.push_front(std::pair(std::vector<char>(size+1),nullptr));
                    MYSQL_BIND b    = {nullptr};
                    b.buffer_type   = MYSQL_TYPE_STRING;
                    b.buffer        = &bufs_.front().first[0];
                    b.buffer_length = size;
                    BindNullFlag(b,null);
                    binds_.push_back(b);
                }
            },

            [&](auto) {
                mysql_stmt_close(stmt_);
                throw std::runtime_error("Illegal data type "
                    + std::to_string(data.index())
                    + " in query buffer for column \"" + v.first + "\"");
            }
        },data);
    }
}
