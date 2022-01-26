/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-01-10     bernard      porting to AM1808
 */

#include <rtthread.h>
#include <rthw.h>
#include <board.h>

#include "cp15.h"
#include "mmu.h"

#ifdef RT_USING_USERSPACE
#include "page.h"
#endif

#define MMU_LEVEL_MASK 0x1ffUL
#define MMU_LEVEL_SHIFT   9
#define MMU_ADDRESS_BITS  39
#define MMU_ADDRESS_MASK  0x0000fffffffff000UL
#define MMU_ATTRIB_MASK   0xfff0000000000ffcUL

#define MMU_TYPE_MASK     3UL
#define MMU_TYPE_USED     1UL
#define MMU_TYPE_BLOCK    1UL
#define MMU_TYPE_TABLE    3UL
#define MMU_TYPE_PAGE     3UL

#define MMU_TBL_BLOCK_2M_LEVEL 2
#define MMU_TBL_PAGE_4k_LEVEL  3
#define MMU_TBL_LEVEL_NR       4

void *_rt_hw_mmu_v2p(rt_mmu_info *mmu_info, void* v_addr);

struct page_table
{
    unsigned long page[512];
};

static struct page_table *__init_page_array;
static unsigned long __page_off = 0UL;
unsigned long get_free_page(void)
{
    if (!__init_page_array)
    {
        __init_page_array = (struct page_table *)(((unsigned long)HEAP_BEGIN + ARCH_PAGE_MASK) & ~(ARCH_PAGE_MASK));
    }
    __page_off++;
    if (__init_page_array + __page_off > (struct page_table *)HEAP_END)
    {
        return 0;
    }
    return (unsigned long)(__init_page_array[__page_off - 1].page);
}

void mmu_memset(char *dst, char v,  size_t len)
{
    while (len--)
    {
        *dst++ = v;
    }
}

static int _map_single_page_2M(unsigned long* lv0_tbl, unsigned long va, unsigned long pa, unsigned long attr)
{
    int level;
    unsigned long* cur_lv_tbl = lv0_tbl;
    unsigned long page;
    unsigned long off;
    int level_shift = MMU_ADDRESS_BITS;

    if (va & ARCH_SECTION_MASK)
    {
        return MMU_MAP_ERROR_VANOTALIGN;
    }
    if (pa & ARCH_SECTION_MASK)
    {
        return MMU_MAP_ERROR_PANOTALIGN;
    }
    for (level = 0; level < MMU_TBL_BLOCK_2M_LEVEL; level++)
    {
        off = (va >> level_shift);
        off &= MMU_LEVEL_MASK;
        if (!(cur_lv_tbl[off] & MMU_TYPE_USED))
        {
            page = get_free_page();
            if (!page)
            {
                return MMU_MAP_ERROR_NOPAGE;
            }
            mmu_memset((char *)page, 0, ARCH_PAGE_SIZE);
            cur_lv_tbl[off] = page | MMU_TYPE_TABLE;
        }
        page = cur_lv_tbl[off];
        if ((page & MMU_TYPE_MASK) == MMU_TYPE_BLOCK)
        {
            //is block! error!
            return MMU_MAP_ERROR_CONFLICT;
        }
        cur_lv_tbl = (unsigned long*)(page & MMU_ADDRESS_MASK);
        level_shift -= MMU_LEVEL_SHIFT;
    }
    attr &= MMU_ATTRIB_MASK;
    pa |= (attr | MMU_TYPE_BLOCK); //block
    off = (va >> ARCH_SECTION_SHIFT);
    off &= MMU_LEVEL_MASK;
    cur_lv_tbl[off] = pa;
    return 0;
}

int armv8_init_map_2M(unsigned long* lv0_tbl, unsigned long va, unsigned long pa, unsigned long count, unsigned long attr)
{
    unsigned long i;
    int ret;

    if (va & ARCH_SECTION_MASK)
    {
        return -1;
    }
    if (pa & ARCH_SECTION_MASK)
    {
        return -1;
    }
    for (i = 0; i < count; i++)
    {
        ret = _map_single_page_2M(lv0_tbl, va, pa, attr);
        va += ARCH_SECTION_SIZE;
        pa += ARCH_SECTION_SIZE;
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

static int _kenrel_map_2M(unsigned long* lv0_tbl, unsigned long va, unsigned long pa, unsigned long attr)
{
    int level;
    unsigned long* cur_lv_tbl = lv0_tbl;
    unsigned long page;
    unsigned long off;
    int level_shift = MMU_ADDRESS_BITS;

    if (va & ARCH_SECTION_MASK)
    {
        return MMU_MAP_ERROR_VANOTALIGN;
    }
    if (pa & ARCH_SECTION_MASK)
    {
        return MMU_MAP_ERROR_PANOTALIGN;
    }
    for (level = 0; level < MMU_TBL_BLOCK_2M_LEVEL; level++)
    {
        off = (va >> level_shift);
        off &= MMU_LEVEL_MASK;
        if (!(cur_lv_tbl[off] & MMU_TYPE_USED))
        {
            page = (unsigned long)rt_pages_alloc(0);
            if (!page)
            {
                return MMU_MAP_ERROR_NOPAGE;
            }
            rt_memset((char *)page, 0, ARCH_PAGE_SIZE);
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)page, ARCH_PAGE_SIZE);
            cur_lv_tbl[off] = (page + PV_OFFSET) | MMU_TYPE_TABLE;
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, cur_lv_tbl + off, sizeof(void *));
        }
        else
        {
            page = cur_lv_tbl[off];
            page &= MMU_ADDRESS_MASK;
            /* page to va */
            page -= PV_OFFSET;
            rt_page_ref_inc((void *)page, 0);
        }
        page = cur_lv_tbl[off];
        if ((page & MMU_TYPE_MASK) == MMU_TYPE_BLOCK)
        {
            //is block! error!
            return MMU_MAP_ERROR_CONFLICT;
        }
        cur_lv_tbl = (unsigned long *)(page & MMU_ADDRESS_MASK);
        cur_lv_tbl = (unsigned long *)((unsigned long)cur_lv_tbl - PV_OFFSET);
        level_shift -= MMU_LEVEL_SHIFT;
    }
    attr &= MMU_ATTRIB_MASK;
    pa |= (attr | MMU_TYPE_BLOCK); //block
    off = (va >> ARCH_SECTION_SHIFT);
    off &= MMU_LEVEL_MASK;
    cur_lv_tbl[off] = pa;
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, cur_lv_tbl + off, sizeof(void *));
    return 0;
}

struct mmu_level_info
{
    unsigned long *pos;
    void *page;
};

static void _kenrel_unmap_4K(unsigned long* lv0_tbl, void* v_addr)
{
    int level;
    unsigned long va = (unsigned long)v_addr;
    unsigned long* cur_lv_tbl = lv0_tbl;
    unsigned long page;
    unsigned long off;
    struct mmu_level_info level_info[4];
    int ref;
    int level_shift = MMU_ADDRESS_BITS;
    unsigned long *pos;

    rt_memset(level_info, 0 , sizeof level_info);
    for (level = 0; level < MMU_TBL_LEVEL_NR; level++)
    {
        off = (va >> level_shift);
        off &= MMU_LEVEL_MASK;
        page = cur_lv_tbl[off];
        if (!(page & MMU_TYPE_USED))
        {
            break;
        }
        if ((page & MMU_TYPE_MASK) == MMU_TYPE_BLOCK)
        {
            break;
        }
        level_info[level].pos = cur_lv_tbl + off;
        cur_lv_tbl = (unsigned long*)(page & MMU_ADDRESS_MASK);
        cur_lv_tbl = (unsigned long *)((unsigned long)cur_lv_tbl - PV_OFFSET);
        level_info[level].page = cur_lv_tbl;
        level_shift -= MMU_LEVEL_SHIFT;
    }

    level = MMU_TBL_PAGE_4k_LEVEL;
    pos = level_info[level].pos;
    if (pos)
    {
        *pos = RT_NULL;
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, pos, sizeof(void *));
    }
    level--;

    while (level >= 0)
    {
        pos = level_info[level].pos;
        if (pos)
        {
            void *cur_page = level_info[level].page;
            ref = rt_page_ref_get(cur_page, 0);
            if (ref == 1)
            {
                *pos = RT_NULL;
                rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, pos, sizeof(void *));
            }
            rt_pages_free(cur_page, 0);
        }
        level--;
    }
    asm volatile("tlbi vae1, %0\ndsb sy"::"r"(v_addr):"memory");
    return;
}

static int _kenrel_map_4K(unsigned long* lv0_tbl, unsigned long va, unsigned long pa, unsigned long attr)
{
    int ret = 0;
    int level;
    unsigned long* cur_lv_tbl = lv0_tbl;
    unsigned long page;
    unsigned long off;
    int level_shift = MMU_ADDRESS_BITS;

    if (va & ARCH_PAGE_MASK)
    {
        return MMU_MAP_ERROR_VANOTALIGN;
    }
    if (pa & ARCH_PAGE_MASK)
    {
        return MMU_MAP_ERROR_PANOTALIGN;
    }
    for (level = 0; level < MMU_TBL_PAGE_4k_LEVEL; level++)
    {
        off = (va >> level_shift);
        off &= MMU_LEVEL_MASK;
        if (!(cur_lv_tbl[off] & MMU_TYPE_USED))
        {
            page = (unsigned long)rt_pages_alloc(0);
            if (!page)
            {
                ret = MMU_MAP_ERROR_NOPAGE;
                goto err;
            }
            rt_memset((void *)page, 0, ARCH_PAGE_SIZE);
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)page, ARCH_PAGE_SIZE);
            cur_lv_tbl[off] = (page + PV_OFFSET) | MMU_TYPE_TABLE;
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, cur_lv_tbl + off, sizeof(void *));
        }
        else
        {
            page = cur_lv_tbl[off];
            page &= MMU_ADDRESS_MASK;
            /* page to va */
            page -= PV_OFFSET;
            rt_page_ref_inc((void *)page, 0);
        }
        page = cur_lv_tbl[off];
        if ((page & MMU_TYPE_MASK) == MMU_TYPE_BLOCK)
        {
            //is block! error!
            ret = MMU_MAP_ERROR_CONFLICT;
            goto err;
        }
        cur_lv_tbl = (unsigned long *)(page & MMU_ADDRESS_MASK);
        cur_lv_tbl = (unsigned long *)((unsigned long)cur_lv_tbl - PV_OFFSET);
        level_shift -= MMU_LEVEL_SHIFT;
    }
    //now is level page
    attr &= MMU_ATTRIB_MASK;
    pa |= (attr | MMU_TYPE_PAGE); //page
    off = (va >> ARCH_PAGE_SHIFT);
    off &= MMU_LEVEL_MASK;
    cur_lv_tbl[off] = pa; //page
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, cur_lv_tbl + off, sizeof(void *));
    return ret;
err:
    _kenrel_unmap_4K(lv0_tbl, (void *)va);
    return ret;
}

int kernel_map_fixed(unsigned long* lv0_tbl, unsigned long va, unsigned long pa, unsigned long count, unsigned long attr)
{
    unsigned long i;
    int ret;
    unsigned long _attr = MMU_MAP_CUSTOM(MMU_AP_KAUN, attr);

    if (va & ARCH_SECTION_MASK)
    {
        return -1;
    }
    if (pa & ARCH_SECTION_MASK)
    {
        return -1;
    }
    for (i = 0; i < count; i++)
    {
        ret = _kenrel_map_2M(lv0_tbl, va, pa, _attr);
        va += ARCH_SECTION_SIZE;
        pa += ARCH_SECTION_SIZE;
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

/************ setting el1 mmu register**************
  MAIR_EL1
  index 0 : memory outer writeback, write/read alloc
  index 1 : memory nocache
  index 2 : device nGnRnE
 *****************************************************/
void mmu_tcr_init(void *tbl0, void *tbl1)
{
    unsigned long val64;

    val64 = 0x00447fUL;
    __asm__ volatile("msr MAIR_EL1, %0\n dsb sy\n"::"r"(val64));

    //TCR_EL1
    val64 = (16UL << 0)  //t0sz 48bit
        | (0x0UL << 6)   //reserved
        | (0x0UL << 7)   //epd0
        | (0x3UL << 8)   //t0 wb cacheable
        | (0x3UL << 10)  //inner shareable
        | (0x2UL << 12)  //t0 outer shareable
        | (0x0UL << 14)  //t0 4K
        | (16UL << 16)   //t1sz 48bit
        | (0x0UL << 22)  //define asid use ttbr0.asid
        | (0x0UL << 23)  //epd1
        | (0x3UL << 24)  //t1 inner wb cacheable
        | (0x3UL << 26)  //t1 outer wb cacheable
        | (0x2UL << 28)  //t1 outer shareable
        | (0x2UL << 30)  //t1 4k
        | (0x1UL << 32)  //001b 64GB PA
        | (0x0UL << 35)  //reserved
        | (0x1UL << 36)  //as: 0:8bit 1:16bit
        | (0x0UL << 37)  //tbi0
        | (0x0UL << 38); //tbi1
    __asm__ volatile("msr TCR_EL1, %0\n"::"r"(val64));
}

/* dump 2nd level page table */
void rt_hw_cpu_dump_page_table_2nd(rt_uint32_t *ptb)
{
}

void rt_hw_cpu_dump_page_table(rt_uint32_t *ptb)
{
}

volatile unsigned long MMUTable[512] __attribute__((aligned(4*1024)));
void rt_hw_mmu_setmtt(unsigned long vaddrStart,
        unsigned long vaddrEnd,
        unsigned long paddrStart,
        unsigned long attr)
{
    unsigned long count;

    if (vaddrStart & ARCH_SECTION_MASK)
    {
        return;
    }
    if (paddrStart & ARCH_SECTION_MASK)
    {
        return;
    }
    if (vaddrStart > vaddrEnd)
    {
        return;
    }
    count = vaddrEnd + 1;
    if (count & ARCH_SECTION_MASK)
    {
        return;
    }
    count -= vaddrStart;
    if (count == 0)
    {
        return;
    }
    count >>= ARCH_SECTION_SHIFT;
    kernel_map_fixed((unsigned long *)MMUTable, vaddrStart, paddrStart, count, attr);
}

static void kernel_mmu_switch(unsigned long tbl)
{
    tbl += PV_OFFSET;
    __asm__ volatile("msr TTBR1_EL1, %0\n dsb sy\nisb"::"r"(tbl):"memory");
    __asm__ volatile("tlbi vmalle1\n dsb sy\nisb":::"memory");
    __asm__ volatile("ic ialluis\n dsb sy\nisb":::"memory");
}

void rt_hw_mmu_setup(struct mem_desc *mdesc, int desc_nr)
{
    /* set page table */
    for(; desc_nr > 0; desc_nr--)
    {
        rt_hw_mmu_setmtt(mdesc->vaddr_start, mdesc->vaddr_end,
                mdesc->paddr_start, mdesc->attr);
        mdesc++;
    }
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)MMUTable, sizeof MMUTable);
    kernel_mmu_switch((unsigned long)MMUTable);
}

/*
   mem map
   */
int rt_hw_mmu_map_init(rt_mmu_info *mmu_info, void* v_address, size_t size, size_t *vtable, size_t pv_off)
{
    rt_base_t level;
    size_t va_s, va_e;

    if (!mmu_info || !vtable)
    {
        return -1;
    }

    va_s = (size_t)v_address;
    va_e = (size_t)v_address + size - 1;

    if ( va_e < va_s)
    {
        return -1;
    }

    va_s >>= ARCH_SECTION_SHIFT;
    va_e >>= ARCH_SECTION_SHIFT;

    if (va_s == 0)
    {
        return -1;
    }

    level = rt_hw_interrupt_disable();

    mmu_info->vtable = vtable;
    mmu_info->vstart = va_s;
    mmu_info->vend = va_e;
    mmu_info->pv_off = pv_off;

    rt_hw_interrupt_enable(level);

    return 0;
}

int rt_hw_mmu_ioremap_init(rt_mmu_info *mmu_info, void* v_address, size_t size)
{
    return 0;
}

static size_t find_vaddr(rt_mmu_info *mmu_info, int pages)
{
    size_t loop_pages;
    size_t va;
    size_t find_va = 0;
    int n = 0;
    size_t i;

    if (!pages)
    {
        return 0;
    }

    if (!mmu_info)
    {
        return 0;
    }

    loop_pages = mmu_info->vend - mmu_info->vstart + 1;
    loop_pages <<= (ARCH_SECTION_SHIFT - ARCH_PAGE_SHIFT);
    va = mmu_info->vstart;
    va <<= ARCH_SECTION_SHIFT;
    for (i = 0; i < loop_pages; i++, va += ARCH_PAGE_SIZE)
    {
        if (_rt_hw_mmu_v2p(mmu_info, (void *)va))
        {
            n = 0;
            find_va = 0;
            continue;
        }
        if (!find_va)
        {
            find_va = va;
        }
        n++;
        if (n >= pages)
        {
            return find_va;
        }
    }
    return 0;
}

#ifdef RT_USING_USERSPACE
static int check_vaddr(rt_mmu_info *mmu_info, void *va, int pages)
{
    size_t loop_va;

    if (!pages)
    {
        return -1;
    }

    if (!mmu_info)
    {
        return -1;
    }

    loop_va = ((size_t)va >> ARCH_SECTION_SHIFT);
    if (loop_va < mmu_info->vstart || loop_va > mmu_info->vend)
    {
        return -1;
    }
    loop_va += ((pages << ARCH_PAGE_SHIFT) >> ARCH_SECTION_SHIFT);
    if (loop_va < mmu_info->vstart || loop_va > mmu_info->vend + 1)
    {
        return -1;
    }

    loop_va = (size_t)va & ~ARCH_PAGE_MASK;
    while (pages--)
    {
        if (_rt_hw_mmu_v2p(mmu_info, (void *)loop_va))
        {
            return -1;
        }
        loop_va += ARCH_PAGE_SIZE;
    }
    return 0;
}
#endif

static void __rt_hw_mmu_unmap(rt_mmu_info *mmu_info, void* v_addr, size_t npages)
{
    size_t loop_va = (size_t)v_addr & ~ARCH_PAGE_MASK;

    if (!mmu_info || !mmu_info->vtable)
    {
        return;
    }

    while (npages--)
    {
        _kenrel_unmap_4K(mmu_info->vtable, (void *)loop_va);
        loop_va += ARCH_PAGE_SIZE;
    }
}

static int __rt_hw_mmu_map(rt_mmu_info *mmu_info, void* v_addr, void* p_addr, size_t npages, size_t attr)
{
    size_t loop_va = (size_t)v_addr & ~ARCH_PAGE_MASK;
    size_t loop_pa = (size_t)p_addr & ~ARCH_PAGE_MASK;

    if (!mmu_info)
    {
        return -1;
    }

    while (npages--)
    {
        _kenrel_map_4K(mmu_info->vtable, loop_va, loop_pa, attr);
        loop_va += ARCH_PAGE_SIZE;
        loop_pa += ARCH_PAGE_SIZE;
    }
    return 0;
}

static void rt_hw_cpu_tlb_invalidate(void)
{
    __asm__ volatile("tlbi vmalle1\n dsb sy\n isb sy\n");
}

#ifdef RT_USING_USERSPACE
void *_rt_hw_mmu_map(rt_mmu_info *mmu_info, void *v_addr, void* p_addr, size_t size, size_t attr)
{
    size_t pa_s, pa_e;
    size_t vaddr;
    int pages;
    int ret;

    if (!size)
    {
        return 0;
    }
    pa_s = (size_t)p_addr;
    pa_e = (size_t)p_addr + size - 1;
    pa_s >>= ARCH_PAGE_SHIFT;
    pa_e >>= ARCH_PAGE_SHIFT;
    pages = pa_e - pa_s + 1;
    if (v_addr)
    {
        vaddr = (size_t)v_addr;
        pa_s = (size_t)p_addr;
        if ((vaddr & ARCH_PAGE_MASK) != (pa_s & ARCH_PAGE_MASK))
        {
            return 0;
        }
        vaddr &= ~ARCH_PAGE_MASK;
        if (check_vaddr(mmu_info, (void*)vaddr, pages) != 0)
        {
            return 0;
        }
    }
    else
    {
        vaddr = find_vaddr(mmu_info, pages);
    }
    if (vaddr) {
        ret = __rt_hw_mmu_map(mmu_info, (void*)vaddr, p_addr, pages, attr);
        if (ret == 0)
        {
            rt_hw_cpu_tlb_invalidate();
            return (void*)(vaddr + ((size_t)p_addr & ARCH_PAGE_MASK));
        }
    }
    return 0;
}
#else
void *_rt_hw_mmu_map(rt_mmu_info *mmu_info, void* p_addr, size_t size, size_t attr)
{
    size_t pa_s, pa_e;
    size_t vaddr;
    int pages;
    int ret;

    pa_s = (size_t)p_addr;
    pa_e = (size_t)p_addr + size - 1;
    pa_s >>= ARCH_PAGE_SHIFT;
    pa_e >>= ARCH_PAGE_SHIFT;
    pages = pa_e - pa_s + 1;
    vaddr = find_vaddr(mmu_info, pages);
    if (vaddr) {
        ret = __rt_hw_mmu_map(mmu_info, (void*)vaddr, p_addr, pages, attr);
        if (ret == 0)
        {
            rt_hw_cpu_tlb_invalidate();
            return (void*)(vaddr + ((size_t)p_addr & ARCH_PAGE_MASK));
        }
    }
    return 0;
}
#endif

#ifdef RT_USING_USERSPACE
static int __rt_hw_mmu_map_auto(rt_mmu_info *mmu_info, void* v_addr, size_t npages, size_t attr)
{
    size_t loop_va = (size_t)v_addr & ~ARCH_PAGE_MASK;
    size_t loop_pa;

    if (!mmu_info)
    {
        return -1;
    }

    while (npages--)
    {
        loop_pa = (size_t)rt_pages_alloc(0);
        if (!loop_pa)
        {
            goto err;
        }
        loop_pa += mmu_info->pv_off;
        _kenrel_map_4K(mmu_info->vtable, loop_va, loop_pa, attr);
        loop_va += ARCH_PAGE_SIZE;
    }
    return 0;
err:
    {
        /* error, unmap and quit */
        int i;
        void *va, *pa;

        va = (void*)((size_t)v_addr & ~ARCH_PAGE_MASK);
        for (i = 0; i < npages; i++)
        {
            pa = rt_hw_mmu_v2p(mmu_info, va);
            pa = (void*)((char*)pa - mmu_info->pv_off);
            rt_pages_free(pa, 0);
            va = (void*)((char*)va + ARCH_PAGE_SIZE);
        }

        __rt_hw_mmu_unmap(mmu_info, v_addr, npages);
        return -1;
    }
}

void *_rt_hw_mmu_map_auto(rt_mmu_info *mmu_info, void *v_addr, size_t size, size_t attr)
{
    size_t vaddr;
    size_t offset;
    int pages;
    int ret;

    if (!size)
    {
        return 0;
    }
    offset = (size_t)v_addr & ARCH_PAGE_MASK;
    size += (offset + ARCH_PAGE_SIZE - 1);
    pages = (size >> ARCH_PAGE_SHIFT);
    if (v_addr)
    {
        vaddr = (size_t)v_addr;
        vaddr &= ~ARCH_PAGE_MASK;
        if (check_vaddr(mmu_info, (void*)vaddr, pages) != 0)
        {
            return 0;
        }
    }
    else
    {
        vaddr = find_vaddr(mmu_info, pages);
    }
    if (vaddr) {
        ret = __rt_hw_mmu_map_auto(mmu_info, (void*)vaddr, pages, attr);
        if (ret == 0)
        {
            rt_hw_cpu_tlb_invalidate();
            return (void*)((char*)vaddr + offset);
        }
    }
    return 0;
}
#endif

void _rt_hw_mmu_unmap(rt_mmu_info *mmu_info, void* v_addr, size_t size)
{
    size_t va_s, va_e;
    int pages;

    va_s = (size_t)v_addr;
    va_e = (size_t)v_addr + size - 1;
    va_s >>= ARCH_PAGE_SHIFT;
    va_e >>= ARCH_PAGE_SHIFT;
    pages = va_e - va_s + 1;
    __rt_hw_mmu_unmap(mmu_info, v_addr, pages);
    rt_hw_cpu_tlb_invalidate();
}

#ifdef RT_USING_USERSPACE
void *rt_hw_mmu_map(rt_mmu_info *mmu_info, void *v_addr, void* p_addr, size_t size, size_t attr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    ret = _rt_hw_mmu_map(mmu_info, v_addr, p_addr, size, attr);
    rt_hw_interrupt_enable(level);
    return ret;
}

void *rt_hw_mmu_map_auto(rt_mmu_info *mmu_info, void *v_addr, size_t size, size_t attr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    ret = _rt_hw_mmu_map_auto(mmu_info, v_addr, size, attr);
    rt_hw_interrupt_enable(level);
    return ret;
}
#endif

void rt_hw_mmu_unmap(rt_mmu_info *mmu_info, void* v_addr, size_t size)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    _rt_hw_mmu_unmap(mmu_info, v_addr, size);
    rt_hw_interrupt_enable(level);
}

void *_rt_hw_mmu_v2p(rt_mmu_info *mmu_info, void* v_addr)
{
    int level;
    unsigned long va = (unsigned long)v_addr;
    unsigned long pa;
    unsigned long* cur_lv_tbl;
    unsigned long page;
    unsigned long off;
    unsigned long off_addr;
    int level_shift = MMU_ADDRESS_BITS;

    if (!mmu_info)
    {
        return (void*)0;
    }
    cur_lv_tbl = mmu_info->vtable;
    for (level = 0; level < MMU_TBL_PAGE_4k_LEVEL; level++)
    {
        off = (va >> level_shift);
        off &= MMU_LEVEL_MASK;
        if (!(cur_lv_tbl[off] & MMU_TYPE_USED))
        {
            return (void *)0;
        }
        page = cur_lv_tbl[off];
        if ((page & MMU_TYPE_MASK) == MMU_TYPE_BLOCK)
        {
            off_addr = va & ((1UL << level_shift) - 1);
            pa = (page & MMU_ADDRESS_MASK);
            pa += off_addr;
            return (void *)pa;
        }
        cur_lv_tbl = (unsigned long*)(page & MMU_ADDRESS_MASK);
        cur_lv_tbl = (unsigned long *)((unsigned long)cur_lv_tbl - PV_OFFSET);
        level_shift -= MMU_LEVEL_SHIFT;
    }
    //now is level MMU_TBL_PAGE_4k_LEVEL
    off = (va >> ARCH_PAGE_SHIFT);
    off &= MMU_LEVEL_MASK;
    page = cur_lv_tbl[off];
    if (!(page & MMU_TYPE_USED))
    {
        return (void *)0;
    }
    pa = (page & MMU_ADDRESS_MASK);
    pa += (va & ARCH_PAGE_MASK);
    return (void *)pa;
}

void *rt_hw_mmu_v2p(rt_mmu_info *mmu_info, void* v_addr)
{
    void *ret;
    rt_base_t level;

    level = rt_hw_interrupt_disable();
    ret = _rt_hw_mmu_v2p(mmu_info, v_addr);
    rt_hw_interrupt_enable(level);
    return ret;
}

#ifdef RT_USING_USERSPACE
void rt_hw_mmu_setup_early(unsigned long *tbl0, unsigned long *tbl1, unsigned long size, unsigned long pv_off)
{
    int ret;
    unsigned long va = KERNEL_VADDR_START;
    unsigned long count = (size + ARCH_SECTION_MASK) >> ARCH_SECTION_SHIFT;
    unsigned long normal_attr = MMU_MAP_CUSTOM(MMU_AP_KAUN, NORMAL_MEM);

    ret = armv8_init_map_2M(tbl1 , va, va + pv_off, count, normal_attr);
    if (ret != 0)
    {
        while (1);
    }
    ret = armv8_init_map_2M(tbl0, va + pv_off, va + pv_off, count, normal_attr);
    if (ret != 0)
    {
        while (1);
    }
}
#endif
