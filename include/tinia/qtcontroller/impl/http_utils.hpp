#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

namespace tinia { namespace qtcontroller { namespace impl {

QString getRequestURI(const QString& request);

bool isGetOrPost(const QString& request);

QString getMimeType(const QString& file);

QMap<QString, QString> decodeGetParameters(const QString& request);

QString getPostContent(const QString& request);

template<unsigned int i>
struct ParseGetHelper {

    template<typename Sequence>
    void parse(Sequence& seq, const QMap<QString, QString>& parameters,
               QStringList& keys);
};

template<>
template<typename Sequence>
void ParseGetHelper<0>::parse(Sequence& seq, const QMap<QString, QString>& parameters,
                              QStringList& keys) {
    if(!parameters.contains(keys[0])) {
        throw std::invalid_argument("Could not find key " + keys[0].toStdString());
    }
    try {
        boost::tuples::get<0>(seq) =
                boost::lexical_cast<typename boost::tuples::element<0, Sequence>::type>(parameters[keys[0]].toStdString());
    } catch(boost::bad_lexical_cast& e) {
        throw std::invalid_argument("Could not parse the value of key "
                                    + keys[0].toStdString());
    }
}

template<unsigned int i>
template<typename Sequence>
void ParseGetHelper<i>::parse(Sequence& seq, const QMap<QString, QString>& parameters,
                                        QStringList& keys) {
    ParseGetHelper<i-1> previous;
    previous.parse(seq, parameters, keys);
    if(!parameters.contains(keys[i])) {
        throw std::invalid_argument("Could not find key " + keys[i].toStdString());
    }
    try {
        boost::tuples::get<i>(seq) =
                boost::lexical_cast<typename boost::tuples::element<i, Sequence>::type>(parameters[keys[i]].toStdString());
    } catch(boost::bad_lexical_cast& e) {
        throw std::invalid_argument("Could not parse the value of key "
                                    + keys[i].toStdString());
    }
}


/**
 * Converts each key found in keyAsString to the type specified by sequence (order
 * the same way as keysAsString
 * \note Sequence must have a default constructor.
 * @param keysAsString the keys to extract, seperated by space.
 */
template<typename Sequence>
Sequence parseGet(const QMap<QString, QString>& parameters, QString keysAsString) {
    Sequence sequence;
    auto keys = keysAsString.split(" ");

    ParseGetHelper<boost::tuples::length<Sequence>::value - 1> helper;
    helper.parse(sequence, parameters, keys);
    return sequence;
}
}}}
