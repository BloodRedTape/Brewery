#include <sqlite3.h>
#include <core/list.hpp>
#include <core/print.hpp>
#include <core/function.hpp>

class QueryResult{
private:
    sqlite3_stmt *const m_Statement;

    bool m_Status;
public:
    QueryResult(sqlite3_stmt *statement):
            m_Statement(statement)
    {
        Reset();
    }

    ~QueryResult(){
        sqlite3_finalize(m_Statement);
    }

    void Next(){
        m_Status = sqlite3_step(m_Statement) == SQLITE_ROW;
    }

    void Reset(){
        sqlite3_reset(m_Statement);
        Next();
    }

    operator bool()const{
        return m_Status;
    }

    int GetColumnInt(size_t index)const{
        return sqlite3_column_int(m_Statement, index);
    }

    const char *GetColumnString(size_t index)const{
        return (const char*)sqlite3_column_text(m_Statement, index);
    }

    float GetColumnFloat(size_t index)const{
        return sqlite3_column_double(m_Statement, index);
    }

    double GetColumnDouble(size_t index)const{
        return sqlite3_column_double(m_Statement, index);
    }
};

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

    const List<std::string> &Lines()const{
        return m_Lines;
    }
};

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
        sqlite3_stmt *sqlite_stmt = nullptr;
        sqlite3_prepare_v2(m_Handle, stmt, -1, &sqlite_stmt, nullptr);

        return {sqlite_stmt};
    }

    size_t Size(const char *table_name){
        auto query = Query({"SELECT * FROM %", table_name});
        size_t counter = 0;

        for(;query; query.Next())
            counter++;

        return counter;
    }
};