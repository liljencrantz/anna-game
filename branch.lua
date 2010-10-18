
function printf(s,...)
   print(s:format(...))
end

function normalize(v)
    scale1 = v[1][1] - v[#v][1]
    off1 = v[#v][1]
    scale2 = (v[1][3]-v[1][2])/2
    off2 = (v[1][3]+v[1][2])/2
    for _,i in ipairs(v) do
        printf(
	   "            {%.2f, %.2f, %.2f},", 
	   1.0-(i[1]-off1)/scale1,
	   -(i[2]-off2)/scale2, 
	(i[3]-off2)/scale2)
    end
end

function branch_define(name, data)
   printf("    tree_section_type_t *%s_tid = malloc(sizeof(tree_section_type_t) + %d*sizeof(float[3]));", name, #data)
   printf("    %s_tid->count=%d;", name, #data)
   printf("    float %s_data[][3] =\n        {", name)
   normalize(data)			
   printf("        }\n    ;")
   printf("    memcpy(%s_tid->data, %s_data, %d*sizeof(float[3]));", name, name, #data)
   print("")
end


branch_define(
   "stem1",
{{447,22,174},
{435,1,185},
{401,2,183},
{374,22,186},
{323,27,164},
{242,35,158},
{159,34,155},
{94,29,154},
{44,23,158}})


branch_define(
   "stem2",
   {{379,11,72},
    {355,1,73},
    {340,14,78},
    {314,1,59},
    {275,24,70},
    {181,24,67},
    {150,32,74},
    {105,22,66},
    {42,30,66},
    {8,20,63}})


branch_define(
   "stem3",
{{391,19,87},
{357,23,86},
{311,17,63},
{272,5,59},
{234,4,57},
{156,17,69},
{50,31,65},
{1,31,74}})

branch_define(
   "root1",
{{144,3,59},
{116,12,53},
{51,5,52},
{2,28,33}})



branch_define(
   "branch1",
{{239,15,67},
{199,22,64},
{108,11,38},
{1,21,43}}
)
branch_define(
   "branch2",
{{158,8,38},
{119,6,29},
{1,12,30}}
)
branch_define(
   "branch3",
{{72,4,22},
{64,5,23},
{55,4,22},
{45,4,19},
{41,4,21},
{24,5,21},
{0,7,19}}
)

branch_define(
   "branch4",
{{190,5,23},
{175,8,23},
{162,7,20},
{145,7,20},
{128,12,19},
{101,14,19},
{67,16,20},
{1,11,18}}
)

branch_define(
   "branch5",
{{105,2,26},
{73,4,26},
{17,9,26},
{0,11,19}}
)



