/**
 * @file    Column.h
 * @ingroup SQLiteCpp
 * @brief   Encapsulation of a Column in a row of the result pointed by the prepared SQLite::Statement.
 *
 * Copyright (c) 2012-2020 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#pragma once

#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Exception.h>

#include <string>
#include <climits> // For INT_MAX


namespace SQLite
{

extern const int INTEGER;   ///< SQLITE_INTEGER
extern const int FLOAT;     ///< SQLITE_FLOAT
extern const int TEXT;      ///< SQLITE_TEXT
extern const int BLOB;      ///< SQLITE_BLOB
extern const int Null;      ///< SQLITE_NULL


/**
 * @brief Encapsulation of a Column in a row of the result pointed by the prepared Statement.
 *
 *  A Column is a particular field of SQLite data in the current row of result
 * of the Statement : it points to a single cell.
 *
 * Its value can be expressed as a text, and, when applicable, as a numeric
 * (integer or floating point) or a binary blob.
 *
 * Thread-safety: a Column object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Column
{
public:
    /**
     * @brief Encapsulation of a Column in a Row of the result.
     *
     * @param[in] aStmtPtr  Shared pointer to the prepared SQLite Statement Object.
     * @param[in] aIndex    Index of the column in the row of result, starting at 0
     */
    Column(Statement::Ptr& aStmtPtr, int aIndex) noexcept;

    // default destructor: the finalization will be done by the destructor of the last shared pointer
    // default copy constructor and assignment operator are perfectly suited :
    // they copy the Statement::Ptr which in turn increments the reference counter.

    /**
     * @brief Return a pointer to the name assigned to this result column (potentially aliased)
     *
     * @see getOriginName() to get original column name (not aliased)
     */
    const char* getName() const noexcept;

    /**
     * @brief Return a string name assigned to this result column (potentially aliased)
     *
     * @see getOriginName() to get original column name (not aliased)
     */
    std::string getNameString() const noexcept;

#ifdef SQLITE_ENABLE_COLUMN_METADATA
    /**
     * @brief Return a pointer to the table column name that is the origin of this result column
     *
     *  Require definition of the SQLITE_ENABLE_COLUMN_METADATA preprocessor macro :
     * - when building the SQLite library itself (which is the case for the Debian libsqlite3 binary for instance),
     * - and also when compiling this wrapper.
     */
    const char* getOriginName() const noexcept;
#endif

    /// Return the integer value of the column.
    int         getInt() const noexcept;
    /// Return the 32bits unsigned integer value of the column (note that SQLite3 does not support unsigned 64bits).
    unsigned    getUInt() const noexcept;
    /// Return the 64bits integer value of the column (note that SQLite3 does not support unsigned 64bits).
    long long   getInt64() const noexcept;
    /// Return the double (64bits float) value of the column
    double      getDouble() const noexcept;
    /**
     * @brief Return a pointer to the text value (NULL terminated string) of the column.
     *
     * @warning The value pointed at is only valid while the statement is valid (ie. not finalized),
     *          thus you must copy it before using it beyond its scope (to a std::string for instance).
     */
    const char* getText(const char* apDefaultValue = "") const noexcept;
    /**
     * @brief Return a pointer to the binary blob value of the column.
     *
     * @warning The value pointed at is only valid while the statement is valid (ie. not finalized),
     *          thus you must copy it before using it beyond its scope (to a std::string for instance).
     */
    const void* getBlob() const noexcept;
    /**
     * @brief Return a std::string for a TEXT or BLOB column.
     *
     * Note this correctly handles strings that contain null bytes.
     */
    std::string getString() const;

    /**
     * @brief Return the type of the value of the column
     *
     * Return either SQLite::INTEGER, SQLite::FLOAT, SQLite::TEXT, SQLite::BLOB, or SQLite::Null.
     *
     * @warning After a type conversion (by a call to a getXxx on a Column of a Yyy type),
     *          the value returned by sqlite3_column_type() is undefined.
     */
    int getType() const noexcept;

    /// Test if the column is an integer type value (meaningful only before any conversion)
    bool isInteger() const noexcept
    {
        return (SQLite::INTEGER == getType());
    }
    /// Test if the column is a floating point type value (meaningful only before any conversion)
    bool isFloat() const noexcept
    {
        return (SQLite::FLOAT == getType());
    }
    /// Test if the column is a text type value (meaningful only before any conversion)
    bool isText() const noexcept
    {
        return (SQLite::TEXT == getType());
    }
    /// Test if the column is a binary blob type value (meaningful only before any conversion)
    bool isBlob() const noexcept
    {
        return (SQLite::BLOB == getType());
    }
    /// Test if the column is NULL (meaningful only before any conversion)
    bool isNull() const noexcept
    {
        return (SQLite::Null == getType());
    }

    /**
     * @brief Return the number of bytes used by the text (or blob) value of the column
     *
     * Return either :
     * - size in bytes (not in characters) of the string returned by getText() without the '\0' terminator
     * - size in bytes of the string representation of the numerical value (integer or double)
     * - size in bytes of the binary blob returned by getBlob()
     * - 0 for a NULL value
     */
    int getBytes() const noexcept;

    /// Alias returning the number of bytes used by the text (or blob) value of the column
    int size() const noexcept
    {
        return getBytes ();
    }

    /// Inline cast operator to char
    operator char() const
    {
        return static_cast<char>(getInt());
    }
    /// Inline cast operator to unsigned char
    operator unsigned char() const
    {
        return static_cast<unsigned char>(getInt());
    }
    /// Inline cast operator to short
    operator short() const
    {
        return static_cast<short>(getInt());
    }
    /// Inline cast operator to unsigned short
    operator unsigned short() const
    {
        return static_cast<unsigned short>(getInt());
    }

    /// Inline cast operator to int
    operator int() const
    {
        return getInt();
    }
    /// Inline cast operator to 32bits unsigned integer
    operator unsigned int() const
    {
        return getUInt();
    }
#if (LONG_MAX == INT_MAX) // 4 bytes "long" type means the data model is ILP32 or LLP64 (Win64 Visual C++ and MinGW)
    /// Inline cast operator to 32bits long
    operator long() const
    {
        return getInt();
    }
    /// Inline cast operator to 32bits unsigned long
    operator unsigned long() const
    {
        return getUInt();
    }
#else // 8 bytes "long" type means the data model is LP64 (Most Unix-like, Windows when using Cygwin; z/OS)
    /// Inline cast operator to 64bits long when the data model of the system is LP64 (Linux 64 bits...)
    operator long() const
    {
        return getInt64();
    }
#endif

    /// Inline cast operator to 64bits integer
    operator long long() const
    {
        return getInt64();
    }
    /// Inline cast operator to double
    operator double() const
    {
        return getDouble();
    }
    /**
     * @brief Inline cast operator to char*
     *
     * @see getText
     */
    operator const char*() const
    {
        return getText();
    }
    /**
     * @brief Inline cast operator to void*
     *
     * @see getBlob
     */
    operator const void*() const
    {
        return getBlob();
    }

    /**
     * @brief Inline cast operator to std::string
     *
     * Handles BLOB or TEXT, which may contain null bytes within
     *
     * @see getString
     */
    operator std::string() const
    {
        return getString();
    }

private:
    Statement::Ptr  mStmtPtr;   ///< Shared Pointer to the prepared SQLite Statement Object
    int             mIndex;     ///< Index of the column in the row of result, starting at 0
};

/**
 * @brief Standard std::ostream text inserter
 *
 * Insert the text value of the Column object, using getText(), into the provided stream.
 *
 * @param[in] aStream   Stream to use
 * @param[in] aColumn   Column object to insert into the provided stream
 *
 * @return  Reference to the stream used
 */
std::ostream& operator<<(std::ostream& aStream, const Column& aColumn);

#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1900) // c++14: Visual Studio 2015

// Create an instance of T from the first N columns, see declaration in Statement.h for full details
template<typename T, int N>
T Statement::getColumns()
{
    checkRow();
    checkIndex(N - 1);
    return getColumns<T>(std::make_integer_sequence<int, N>{});
}

// Helper function called by getColums<typename T, int N>
template<typename T, const int... Is>
T Statement::getColumns(const std::integer_sequence<int, Is...>)
{
    return T{Column(mStmtPtr, Is)...};
}

#endif

}  // namespace SQLite
