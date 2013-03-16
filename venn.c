
struct varray {
    size_t cnt;
    char **vals;
};
typedef struct varray varray;

static void varray_copy_shallow(vararray *dst, const varray *src)
{
    size_t valbytes = src->cnt * sizeof *src->vals;
    dst->cnt = src->cnt;
    dst->vals = malloc(valbytes);
    memcpy(dst->vals, src->vals, valbytes);
}

static int cmp(const char *a, const char *b)
{
    return strcmp(a, b);
}

static void varray_sort(varray *a)
{
    qsort(a, a->cnt, sizeof *a->vals, cmp);
}

static void until_equal(
        const varray *a,
        const varray *b,
        varray *a_only,
        size_t *ai)
{
    int cmp;
    while (*ai < a->cnt)
    {
        cmp = strcmp(a->vals[ai], b->vals[bi]);
        if (cmp >= 0)
            return;
        
        (*ai)++;
    }
}

static void intersection(const varray *a, const varray *b,
                         varray *ab,
                         varray *only_a,
                         varray *only_b)
{
    vararray a_copy, b_copy;
    varray_copy_shallow(&a_copy, a);
    varray_copy_shallow(&b_copy, b);
    varray_sort(&a_copy);
    varray_sort(&b_copy);
    {
        size_t ai = 0,
               bi = 0;
        while (ai < a_copy.cnt && bi < b_copy.cnt)
        {
            int cmp;
            cmp = strcmp(a_copy->vals[ai], b_copy->vals[bi]);
            if (cmp == 0)
                ab
            else if (cmp < 0)
                a_only
            else
                b_only
        }
    }
    free(a_copy.vals);
    free(b_copy.vals);
}

int main(void)
{
    return 0;
}
