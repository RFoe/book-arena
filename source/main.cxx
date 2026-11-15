#include <__policy/__asx.hpp>
#include <__policy/__cn.hpp>

auto main() -> int {
    {
        using namespace __policy::_CN_; // NOLINT
        __v0::__book __x;
        __v1::__book __y;
        __x.__apply(
            __insert{
                .id     = 0,
                .side   = __xgr::__side_t::_S_ask,
                .price  = __price_t{232},
                .volume = 222,
            });
        __y.__apply(
            __insert{
                .id     = 0,
                .side   = __xgr::__side_t::_S_ask,
                .price  = __price_t{232},
                .volume = 222,
            });
    }
    {
        using namespace __policy::_ASX_; // NOLINT
        __v0::__book __x;
        __v1::__book __y;
        __x.__apply(
            __insert{
                .id       = 0,
                .side     = __xgr::__side_t::_S_ask,
                .volume   = 222,
                .position = 32,
                .price    = __price_t{232},
            });
        __y.__apply(
            __insert{
                .id       = 0,
                .side     = __xgr::__side_t::_S_ask,
                .volume   = 222,
                .position = 32,
                .price    = __price_t{232},
            });
    }
}
