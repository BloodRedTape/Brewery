#include <sqlite3.h>

class Stmt{
private:
    static constexpr size_t s_BufferSize = 4096;
    char m_Buffer[s_BufferSize];
    int m_Written = 0;
public:
    template<typename ...Args>
    Stmt(const char *fmt, Args&&...args){
        const auto writer = [](char ch, void *user_data){
            Stmt *stmt = (Stmt*)user_data;
            stmt->m_Buffer[stmt->m_Written++] = ch;
        };

        WriterPrint(writer, &m_Buffer, fmt, Forward<Args>(args)...);
    }

    operator const char *()const{
        return m_Buffer;
    }
};

class DatabaseLogger{
private:
    List<std::string> m_Lines;
public:

    template<typename ...ArgsType>
    void Log(const char *fmt, ArgsType&&...args){
        std::string buffer;

        const auto writer = [](char ch, void *data){
            std::string &buffer = *(std::string*)data;
            buffer.push_back(ch);
        };

        WriterPrint(writer, &buffer, fmt, Forward<ArgsType>(args)...);

        m_Lines.Add(Move(buffer));
    }

    void Log(const class QueryResult &result);

    const List<std::string> &Lines()const{
        return m_Lines;
    }

    void Clear(){
        m_Lines.Clear();
    }
};

class QueryResult{
private:
    sqlite3 *m_Database;
    sqlite3_stmt *m_Query;
    DatabaseLogger &m_Logger;
    Stmt m_Statement;
    bool m_Status = false;
private:
    friend class Database;
    QueryResult(sqlite3 *db, const Stmt &stmt, DatabaseLogger &logger):
        m_Database(db),
        m_Statement(stmt),
        m_Logger(logger)
    {
        if(sqlite3_prepare_v2(m_Database, stmt, -1, &m_Query, nullptr) != SQLITE_OK){
             m_Logger.Log("[SQLite]: %", sqlite3_errmsg(m_Database));
        }else {
            Reset();
        }
    }
public:
    QueryResult(const QueryResult &other):
            QueryResult(other.m_Database, other.m_Statement, other.m_Logger)
    {}

    ~QueryResult(){
        sqlite3_finalize(m_Query);
    }
    QueryResult &operator=(const QueryResult &other){
        this->~QueryResult();
        new (this) QueryResult(other);
        return *this;
    }

    void Next(){
        m_Status = sqlite3_step(m_Query) == SQLITE_ROW;
    }

    void Reset(){
        sqlite3_reset(m_Query);
        Next();
    }

    operator bool()const{
        return m_Status;
    }

    int GetColumnInt(size_t index)const{
        return sqlite3_column_int(m_Query, index);
    }

    const char *GetColumnString(size_t index)const{
        return (const char*)sqlite3_column_text(m_Query, index);
    }

    float GetColumnFloat(size_t index)const{
        return sqlite3_column_double(m_Query, index);
    }

    double GetColumnDouble(size_t index)const{
        return sqlite3_column_double(m_Query, index);
    }

    const char *GetColumnName(size_t index)const{
        return sqlite3_column_name(m_Query, index);
    }

    size_t GetColumnCount()const{
        return sqlite3_column_count(m_Query);
    }
};

void DatabaseLogger::Log(const class QueryResult &query){
    auto result = query;
    Log("[QueryResult]:");
    for(; result; result.Next()) {
        std::stringstream string;
        for(int i = 0;; i++){
            string << result.GetColumnString(i);

            if (i == result.GetColumnCount() - 1)break;

            string << std::setw(20);
        }
        Log(string.str().c_str());
    }
}


class Database{
private:
    sqlite3 *m_Handle = nullptr;
    DatabaseLogger &m_Logger;

    using CallbackType = Function<void(int, char**, char**)>;
public:

    Database(const char *filepath, DatabaseLogger &logger):
            m_Logger(logger)
    {
        sqlite3_open(filepath, &m_Handle);
    }

    ~Database(){
        sqlite3_close(m_Handle);
    }

    bool Execute(const Stmt &stmt){
        char *message = nullptr;
        if(sqlite3_exec(m_Handle, stmt, nullptr, nullptr, &message) != SQLITE_OK){
            m_Logger.Log("[SQLite]: %", message);
            sqlite3_free(message);
            return false;
        }

        return true;
    }

    bool Execute(const Stmt &stmt, int (*callback)(void *usr, int, char **, char **), void *usr){
        char *message = nullptr;
        if(sqlite3_exec(m_Handle, stmt, callback, usr, &message) != SQLITE_OK){
            m_Logger.Log("[SQLite]: %", message);
            sqlite3_free(message);
            return false;
        }

        return true;
    }

    QueryResult Query(const Stmt &stmt){
        return {m_Handle, stmt, m_Logger};
    }

    size_t Size(const char *table_name){
        auto query = Query({"SELECT * FROM %", table_name});
        size_t counter = 0;

        for(;query; query.Next())
            counter++;

        return counter;
    }
};