#include "inverse_bin_fun.hpp"

namespace pomagma
{

#define POMAGMA_ASSERT_CONTAINS(POMAGMA_set, POMAGMA_x, POMAGMA_y, POMAGMA_z)\
    POMAGMA_ASSERT(contains(POMAGMA_set, POMAGMA_x, POMAGMA_y, POMAGMA_z),\
    #POMAGMA_set " is missing " #POMAGMA_x ", " #POMAGMA_y ", " #POMAGMA_z)

void inverse_bin_fun::validate (dense_bin_fun & fun)
{
    POMAGMA_INFO("Validating inverse_bin_fun");

    for (dense_bin_fun::lr_iterator iter(fun); iter.ok(); iter.next()) {
        oid_t lhs = iter.lhs();
        oid_t rhs = iter.rhs();
        oid_t val = iter.value();

        POMAGMA_ASSERT_CONTAINS(m_Vlr_data, val, lhs, rhs);
        POMAGMA_ASSERT_CONTAINS(m_VLr_data, val, lhs, rhs);
        POMAGMA_ASSERT_CONTAINS(m_VRl_data, val, rhs, lhs);
    }

    for (oid_t val = 1; val <= m_item_dim; ++val) {
        for (auto lr : m_Vlr_data[val]) {
            oid_t lhs = lr.first;
            oid_t rhs = lr.second;
            POMAGMA_ASSERT_EQ(fun.get_value(lhs, rhs), val);
        }
    }

    for (auto VL_iter : m_VLr_data) {
        oid_t val = VL_iter.first.first;
        oid_t lhs = VL_iter.first.second;
        for (oid_t rhs : VL_iter.second) {
            POMAGMA_ASSERT_EQ(fun.get_value(lhs, rhs), val);
        }
    }

    for (auto VR_iter : m_VRl_data) {
        oid_t val = VR_iter.first.first;
        oid_t rhs = VR_iter.first.second;
        for (oid_t lhs : VR_iter.second) {
            POMAGMA_ASSERT_EQ(fun.get_value(lhs, rhs), val);
        }
    }
}

} // namespace pomagma
