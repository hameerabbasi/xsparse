#ifndef XSPARSE_COORDINATE_ITERATE_HPP
#define XSPARSE_COORDINATE_ITERATE_HPP

#include <iterator>
#include <tuple>
#include <optional>
#include <type_traits>
#include <utility>

#include <xtl/xiterator_base.hpp>

#include <xsparse/util/template_utils.hpp>
#include <xsparse/util/base_traits.hpp>


namespace xsparse::level_capabilities
{
    template <template <class...> class T, class LowerLevels, class... Opts>
    class coordinate_value_iterate;

    template <template <class...> class T, class... LowerLevels, class... Opts>
    class coordinate_value_iterate<T, std::tuple<LowerLevels...>, Opts...>
    {
        using BaseTraits = util::base_traits<T, std::tuple<LowerLevels...>, Opts...>;

    public:
        class iteration_helper
        {
            static_assert(std::is_nothrow_invocable_r_v<std::optional<typename BaseTraits::PK>,
                                                        decltype(&BaseTraits::Level::coord_access),
                                                        typename BaseTraits::Level&,
                                                        typename BaseTraits::PKM1,
                                                        typename BaseTraits::I,
                                                        typename BaseTraits::IK>);

            static_assert(std::is_nothrow_invocable_r_v<
                          std::pair<typename BaseTraits::IK, typename BaseTraits::IK>,
                          decltype(&BaseTraits::Level::coord_bounds),
                          typename BaseTraits::Level&,
                          typename BaseTraits::I>);

        private:
            typename BaseTraits::Level const& m_level;
            typename BaseTraits::I const m_i;
            typename BaseTraits::PKM1 const m_pkm1;
            typename BaseTraits::IK m_ik_begin, m_ik_end;

        public:
            class iterator;
            using value_type = typename BaseTraits::PK;
            using difference_type = typename std::make_signed_t<typename BaseTraits::IK>;
            using key_type = typename BaseTraits::IK;
            using pointer = typename BaseTraits::PK*;
            using reference = std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
            using iterator_type = iterator;

            class iterator : public xtl::xrandom_access_iterator_base2<iteration_helper>
            {
            private:
                iteration_helper const& m_iterHelper;
                typename BaseTraits::IK m_ik;

            public:
                explicit inline iterator(iteration_helper const& iterationHelper,
                                         typename BaseTraits::IK ik) noexcept
                    : m_iterHelper(iterationHelper)
                    , m_ik(ik)
                {
                }

                inline std::tuple<typename BaseTraits::IK, typename BaseTraits::PK> operator*()
                    const noexcept
                {
                    auto pk = m_iterHelper.m_level.coord_access(
                        m_iterHelper.m_pkm1, m_iterHelper.m_i, m_ik);
                    return { m_ik, pk.value() };
                }

                inline iterator& operator++() noexcept
                {
                    ++m_ik;
                    return *this;
                }

                inline iterator& operator--() noexcept
                {
                    --m_ik;
                    return *this;
                }

                inline iterator& operator+=(difference_type n) noexcept
                {
                    m_ik += n;
                    return *this;
                }

                inline iterator& operator-=(difference_type n) noexcept
                {
                    m_ik -= n;
                    return *this;
                }

                inline difference_type operator-(iterator const& other)
                {
                    return static_cast<difference_type>(m_ik)
                           - static_cast<difference_type>(other.m_ik);
                }

                inline bool operator==(iterator const& other) const noexcept
                {
                    return m_ik == other.m_ik;
                }

                inline bool operator<(iterator const& other) const noexcept
                {
                    return m_ik < other.m_ik;
                }
            };

            explicit inline iteration_helper(typename BaseTraits::Level const& level,
                                             typename BaseTraits::I const i,
                                             typename BaseTraits::PKM1 const pkm1) noexcept
                : m_level(level)
                , m_i(i)
                , m_pkm1(pkm1)
            {
                auto p = level.coord_bounds(i);
                m_ik_begin = p.first;
                m_ik_end = p.second;
            }

            inline iterator_type begin() const noexcept
            {
                return iterator_type{ *this, m_ik_begin };
            }

            inline iterator_type end() const noexcept
            {
                return iterator_type{ *this, m_ik_end };
            }
        };

        iteration_helper iter_helper(typename BaseTraits::I i, typename BaseTraits::PKM1 pkm1)
        /*Create an instance of iteration_helper that manages the iteration process.*/
        {
            return iteration_helper{ *static_cast<typename BaseTraits::Level*>(this), i, pkm1 };
        }
    };

    template <template <class...> class T, class IK, class PK, class... LowerLevels>
    class coordinate_position_iterate
    {
        using BaseTraits = util::base_traits<T, IK, PK, LowerLevels...>;
        
    public:
        class iteration_helper
        {
            static_assert(std::is_nothrow_invocable_r_v<typename BaseTraits::IK,
                                                        decltype(&BaseTraits::Level::pos_access),
                                                        typename BaseTraits::Level&,
                                                        typename BaseTraits::PK,
                                                        typename BaseTraits::I>);

            static_assert(std::is_nothrow_invocable_r_v<
                          std::pair<typename BaseTraits::PK, typename BaseTraits::PK>,
                          decltype(&BaseTraits::Level::pos_bounds),
                          typename BaseTraits::Level&,
                          typename BaseTraits::PKM1>);

        private:
            typename BaseTraits::Level const& m_level;
            typename BaseTraits::I const m_i;
            typename BaseTraits::PK m_pk_begin, m_pk_end;

        public:
            class iterator;
            using value_type = typename BaseTraits::PK;
            using difference_type = typename std::make_signed_t<typename BaseTraits::PK>;
            using key_type = typename BaseTraits::IK;
            using pointer = typename BaseTraits::PK*;
            using reference = std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
            using iterator_type = iterator;

            class iterator : public xtl::xrandom_access_iterator_base2<iteration_helper>
            {
            private:
                iteration_helper const& m_iterHelper;
                typename BaseTraits::PK m_pk;

            public:
                explicit inline iterator(iteration_helper const& iterationHelper,
                                         typename BaseTraits::PK pk) noexcept
                    : m_iterHelper(iterationHelper)
                    , m_pk(pk)
                {
                }

                inline std::tuple<typename BaseTraits::IK, typename BaseTraits::PK> operator*()
                    const noexcept
                {
                    auto ik = m_iterHelper.m_level.pos_access(m_pk, m_iterHelper.m_i);
                    return { ik, m_pk };
                }

                inline iterator& operator++() noexcept
                {
                    ++m_pk;
                    return *this;
                }

                inline iterator& operator--() noexcept
                {
                    --m_pk;
                    return *this;
                }

                inline iterator& operator+=(difference_type n) noexcept
                {
                    m_pk += n;
                    return *this;
                }

                inline iterator& operator-=(difference_type n) noexcept
                {
                    m_pk -= n;
                    return *this;
                }

                inline difference_type operator-(iterator const& other)
                {
                    return static_cast<difference_type>(m_pk)
                           - static_cast<difference_type>(other.m_pk);
                }

                inline bool operator==(iterator const& other) const noexcept
                {
                    return m_pk == other.m_pk;
                }

                inline bool operator<(iterator const& other) const noexcept
                {
                    return m_pk < other.m_pk;
                }
            };

            explicit inline iteration_helper(typename BaseTraits::Level const& level,
                                             typename BaseTraits::I const i,
                                             typename BaseTraits::PKM1 const pkm1) noexcept
                : m_level(level)
                , m_i(i)
            {
                auto p = level.pos_bounds(pkm1);
                m_pk_begin = p.first;
                m_pk_end = p.second;
            }

            inline iterator_type begin() const noexcept
            {
                return iterator{ *this, m_pk_begin };
            }

            inline iterator_type end() const noexcept
            {
                return iterator{ *this, m_pk_end };
            }
        };

        iteration_helper iter_helper(typename BaseTraits::I i, typename BaseTraits::PKM1 pkm1)
        {
            return iteration_helper{ *static_cast<typename BaseTraits::Level*>(this), i, pkm1 };
        }
    };

    template <template <class...> class T, class IK, class PK, class... LowerLevels>
    class locate_position_iterate
    {
        using BaseTraits = util::base_traits<T, IK, PK, LowerLevels...>;

        public:
            class iteration_helper
            {
                static_assert(std::is_nothrow_invocable_r_v<std::optional<typename BaseTraits::PK>,
                                                            decltype(&BaseTraits::Level::locate),
                                                            typename BaseTraits::Level&,
                                                            typename BaseTraits::PKM1,
                                                            typename BaseTraits::IK>);

            private:
                typename BaseTraits::Level const& m_level;
                typename BaseTraits::I const m_i;
                typename BaseTraits::PKM1 const m_pkm1;
                typename BaseTraits::IK m_ik_begin, m_ik_end;

            public:
                class iterator;
                using value_type =
                    typename std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
                using difference_type = typename std::make_signed_t<typename BaseTraits::PK>;
                using pointer =
                    typename std::pair<typename BaseTraits::IK, typename BaseTraits::PK>*;
                using reference = std::pair<typename BaseTraits::IK, typename BaseTraits::PK>;
                using iterator_type = iterator;

                class iterator : public xtl::xbidirectional_iterator_base2<iteration_helper>
                {

                private:
                    iteration_helper const& m_iterHelper;
                    typename BaseTraits::IK m_ik;
                    // using wrapped_iterator_type = typename ContainerTraits::template Map<
                    //     typename BaseTraits::IK,
                    //     typename BaseTraits::PK>::const_iterator;
                    // wrapped_iterator_type m_iterHelper;

                public:
                    explicit inline iterator(iteration_helper iterationHelper) noexcept
                        : m_iterHelper(iterationHelper)
                        , m_ik(ik)
                    {
                    }

                    inline std::tuple<typename BaseTraits::IK, typename BaseTraits::PK> operator*()
                        const noexcept
                    {
                        return { m_iterHelper->first, m_iterHelper->second };
                    }

                    inline bool operator==(const iterator& other) const noexcept
                    {
                        return m_iterHelper == other.m_iterHelper;
                    }

                    inline iterator& operator++() noexcept
                    {
                        ++m_iterHelper;
                        return *this;
                    }

                    inline iterator& operator--() noexcept
                    {
                        --m_iterHelper;
                        return *this;
                    }
                };

                explicit inline iteration_helper(typename BaseTraits::IK const& ik
                                                 typename BaseTraits::PK const& pk map) noexcept
                    : m_map(map)
                {
                }

                inline iterator_type begin() const noexcept
                {
                    return iterator_type{ m_map.begin() };
                }

                inline iterator_type end() const noexcept
                {
                    return iterator_type{ m_map.end() };
                }
            };

            iteration_helper iter_helper(typename BaseTraits::PKM1 pkm1)
            {
                return iteration_helper{ this->m_crd[pkm1] };
            }
    }
}

#endif  // XSPARSE_COORDINATE_ITERATE_HPP