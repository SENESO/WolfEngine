
local SpiritTypeID= {
Amount=19,
[0] = 4200001,
[1] = 4200002,
[2] = 4200003,
[3] = 4200004,
[4] = 4200005,
[5] = 4200006,
[6] = 4200007,
[7] = 4200008,
[8] = 4200009,
[9] = 4200010,
[10] = 4200011,
[11] = 4200012,
[12] = 4200013,
[13] = 4200014,
[14] = 4200015,
[15] = 4200016,
[16] = 4200017,
[17] = 4200018,
[18] = 4200019,
}

local SpiritTipsInfo= {
["minIcon"] = {x = -20, y = 0, w = 20, h = 20},
["tipsbg"] = {x = -0, y = -10, w = 20, h = 16},
["effect"] = {x = 35, y = 60},
["bageffect"] = {x = 23, y = 23, w = 45, h = 45},
[4200001] = {effect = "DragonSoul_green", bageffect = "DS_green_box_no", tipsbg = "DragonSoulTips2BG", battle = 1},
[4200002] = {effect = "DragonSoul_green", bageffect = "DS_green_box_blue", tipsbg = "DragonSoulTips2BG", battle = 1},
[4200003] = {effect = "DragonSoul_green", bageffect = "DS_green_box_purple", tipsbg = "DragonSoulTips2BG", battle = 1},
[4200004] = {effect = "DragonSoul_green", bageffect = "DS_green_box_red", tipsbg = "DragonSoulTips2BG", battle = 2},
[4200005] = {effect = "DragonSoul_green", bageffect = "DS_green_box_orange", tipsbg = "DragonSoulTips2BG", battle = 3},
[4200006] = {effect = "DragonSoul_blue", bageffect = "DS_blue_box_no", tipsbg = "DragonSoulTips3BG", battle = 4},
[4200007] = {effect = "DragonSoul_blue", bageffect = "DS_blue_box_blue", tipsbg = "DragonSoulTips3BG", battle = 5},
[4200008] = {effect = "DragonSoul_blue", bageffect = "DS_blue_box_purple", tipsbg = "DragonSoulTips3BG", battle = 6},
[4200009] = {effect = "DragonSoul_blue", bageffect = "DS_blue_box_red", tipsbg = "DragonSoulTips3BG", battle = 7},
[4200010] = {effect = "DragonSoul_blue", bageffect = "DS_blue_box_orange", tipsbg = "DragonSoulTips3BG", battle = 8},
[4200011] = {effect = "DragonSoul_red", bageffect = "DS_red_box_no", tipsbg = "DragonSoulTips4BG", battle = 9},
[4200012] = {effect = "DragonSoul_red", bageffect = "DS_red_box_blue", tipsbg = "DragonSoulTips4BG", battle = 10},
[4200013] = {effect = "DragonSoul_red", bageffect = "DS_red_box_purple", tipsbg = "DragonSoulTips4BG", battle = 11},
[4200014] = {effect = "DragonSoul_red", bageffect = "DS_red_box_red", tipsbg = "DragonSoulTips4BG", battle = 12},
[4200015] = {effect = "DragonSoul_red", bageffect = "DS_red_box_orange", tipsbg = "DragonSoulTips4BG", battle = 13},
[4200016] = {effect = "DragonSoul_gold1", bageffect = "DS_gold1", tipsbg = "DragonSoulTips5BG", battle = 14},
[4200017] = {effect = "DragonSoul_gold2", bageffect = "DS_gold2", tipsbg = "DragonSoulTips5BG", battle = 15},
[4200018] = {effect = "DragonSoul_gold3", bageffect = "DS_gold3", tipsbg = "DragonSoulTips6BG", battle = 16},
[4200019] = {effect = "DragonSoul_gold3", bageffect = "DS_gold3", tipsbg = "DragonSoulTips6BG", battle = 16},
}

local EffectOffset = {
    ["DragonSoul_fjcg"] = { x=380, y=200 },
    ["DragonSoul_box"] = { x=500, y=270 },
    ["DragonSoul_title"] = { x=500, y=260 },
}

local UpRankInfo= {
[1] = { sound="Sound/ds_fjcg.wav", effect="DragonSoul_sjcg", },
[2] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[3] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[4] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[5] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[6] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[7] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[8] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[9] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[10] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },

}

local function GetPrizeInfo(rank, key, default)
    local tRank = UpRankInfo[rank]

    if (tRank == nil) then
        return default
    end

    local ret = tRank[key]

    if (ret == nil) then
        return default
    end

    return ret
end

local UpRankInfoEx= {
[1] = { sound="Sound/ds_fjcg.wav", effect="DragonSoul_sjcg", },
[2] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[3] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[4] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[5] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[6] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[7] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[8] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },
[9] = { sound="Sound/ds_hccg.wav", effect="DragonSoul_sjcg", },
[10] = { sound="Sound/ds_lhjx.wav", effect="DragonSoulAwaken", },

}

local function GetPrizeInfoEx(rank, key, default)
    local tRank = UpRankInfoEx[rank]

    if (tRank == nil) then
        return default
    end

    local ret = tRank[key]

    if (ret == nil) then
        return default
    end

    return ret
end

local DlgInfo =
{
["DlgQLUpLev"] = {
    ProgressTime=2700,
    MaxLevItem=4200018,
    turnEffX=509,
    turnEffY=252
},
["DlgQLUpLevEx"] = {
    ProgressTime=2700,
    MaxLevItem=4200018,
    turnEffect="DragonSoul_noturn2",
    turnEffX=300,
    turnEffY=162,
    BoundLevItemtype=4200010
},
["DlgQLPrizeTip"] = {
    ExsitTime=1500,
    StandTime=1500,
    MoveSpeed=0,
},
["DlgEmbedQL"] = {
    EmbedEffect="dz_set",
    EmbedEffX=469,
    EmbedEffY=198,
    RemoveEffect="dz_dsremove",
    RemoveEffX=346,
    RemoveEffY=189,

    AttriEff0 ="dz_fighting",
    AttriEff1 ="dz_zbzjxg",
    AttriEff2 ="dz_zzwsjc",
    AttriEff3 ="dz_zzwsjm",
    AttriEff4 ="dz_zzfsjc",
    AttriEff5 ="dz_zzfsjm",
    AddEff = "dz_redNumAdd",
    SubEff = "dz_greenNumReduce",
    UpEffX = 310,
    UpEffY = 0,
    UpEffStrX = 210,
    UpEffStrY = 140,
    NumEffX = 360,
    NumEffY = 140,
    NumSpaceX = 36,
    EffSpaceY = 40,
    EffPerSpaceX = 0,

    AttrBgX=145,
    AttrBgY=243,
    AttrBgW=300,
    AttrBgH=81,
    AttrBgColor=0xbb111111,
},
["DlgQLReforge"] = {
    ProgressTime=2700,
    turnEffX=509,
    turnEffY=252,
	enable=1,
},
}

local QLTotalLevelEffectInfo = {
{
    level = 35,
    effect = "Aspirit_bk",
},
{
    level = 70,
    effect = "Aspirit_bkhigh",
},
{
    level = 95,
    effect = "aspirit_lowlong",
},
{
    level = 126,
    effect = "aspirit_highlong",
},
}

local function GetEffectByQLTotalLevel(QLTotalLevel)
    local maxLevel = 0
    local ret = ""
    
    for i,v in ipairs(QLTotalLevelEffectInfo) do
        if (QLTotalLevel >= v.level and v.level > maxLevel) then
            maxLevel = v.level
            ret = v.effect
        end
    end
    
    return ret
end

local function GetQLEffectMinLev()
    local minLev = QLTotalLevelEffectInfo[1].level
    
    for i,v in ipairs(QLTotalLevelEffectInfo) do
        if (v.level < minLev) then
            minLev = v.level
        end
    end
    
    return minLev
end

local function GetDlgInfo(dlg, key, default, bigUI)
    local tDlg = DlgInfo[dlg]

    if (tDlg == nil) then
        return default
    end

    local ret = tDlg[key]

    if (ret == nil) then
        return default
    end

    return ret
end

local function GetEffectOffset(Effect, Axis)
    return EffectOffset[Effect][Axis]
end

local function GetSpiritAmount()
    return SpiritTypeID.Amount
end

local function GetSpiritTypeID(Index)
    return SpiritTypeID[Index]
end

local function GetSpiritTipsInfo(TypeID, Axis)
    return SpiritTipsInfo[TypeID][Axis]
end

local function GetSpiritTipsOffset(NameType,Axis)
    return SpiritTipsInfo[NameType][Axis]
end

local ReforgeInfo= {
["Amount"]=8,
[4200001] = {maxlevitem=4200002,is_uplev=1,is_visible=1,drop_exp=5},
[4200002] = {maxlevitem=4200003,is_uplev=1,is_visible=1,drop_exp=10},
[4200003] = {maxlevitem=4200004,is_uplev=1,is_visible=1,drop_exp=20},
[4200004] = {maxlevitem=4200005,is_uplev=1,is_visible=1,drop_exp=30},
[4200005] = {maxlevitem=4200006,is_uplev=1,is_visible=1,drop_exp=50},
[4200006] = {maxlevitem=4200007,is_uplev=1,is_visible=1,drop_exp=100},
[4200007] = {maxlevitem=4200008,is_uplev=1,is_visible=1,drop_exp=150},
[4200008] = {maxlevitem=4200009,is_uplev=1,is_visible=1,drop_exp=200},
[4200009] = {maxlevitem=4200010,is_uplev=1,is_visible=1,drop_exp=250},
[4200010] = {maxlevitem=4200011,is_uplev=1,is_visible=1,drop_exp=300},
[4200011] = {maxlevitem=4200012,is_uplev=1,is_visible=1,drop_exp=500},
[4200012] = {maxlevitem=4200013,is_uplev=1,is_visible=1,drop_exp=800},
[4200013] = {maxlevitem=4200014,is_uplev=1,is_visible=1,drop_exp=1200},
[4200014] = {maxlevitem=4200015,is_uplev=1,is_visible=1,drop_exp=2000},
[4200015] = {maxlevitem=4200016,is_uplev=1,is_visible=1,drop_exp=3000},
[4200016] = {maxlevitem=4200017,is_uplev=1,is_visible=1,drop_exp=5000},
[4200017] = {maxlevitem=4200018,is_uplev=1,is_visible=1,drop_exp=8000},
}

local ReforgeEffectInfo= {
["uplev"] = { sound="Sound/ds_fjcg.wav", effect="DragonSoul_czdj",effectX=296,effectY=196 },
["awaken"] = { sound="Sound/ds_lhjx.wav", effect="DragonSoul_czgj",effectX=296,effectY=196 }
}

local function GetReforgeInfo(typeid, key, default)
	local info = ReforgeInfo[typeid]

	if (info == nil) then
		return default
	end

	local ret = info[key]
	
	if (ret == nil) then
		return default
	end
	
	return ret
end

local function GetReforgeEffectInfo(effect_key, key, default)
    local tEffect = ReforgeEffectInfo[effect_key]
    if (tEffect == nil) then
        return default
    end

    local ret = tEffect[key]

    if (ret == nil) then
        return default
    end

    return ret
end

local function Unload()
    SpiritTypeID = nil
    SpiritTipsAni = nil
    UpRankInfo = nil
    DlgInfo = nil
    UpRankInfoEx = nil
end

local Spirit = {
    GetEffectOffset = GetEffectOffset,
    GetSpiritAmount = GetSpiritAmount,
    GetSpiritTypeID = GetSpiritTypeID,
    GetSpiritTipsInfo = GetSpiritTipsInfo,
    GetSpiritTipsOffset = GetSpiritTipsOffset,
    GetPrizeInfo = GetPrizeInfo,
    GetDlgInfo = GetDlgInfo,
    GetEffectByQLTotalLevel = GetEffectByQLTotalLevel,
    GetQLEffectMinLev = GetQLEffectMinLev,
    GetPrizeInfoEx = GetPrizeInfoEx,
	GetReforgeInfo = GetReforgeInfo,
	GetReforgeEffectInfo = GetReforgeEffectInfo,
    Unload = Unload,
}

Spirit_GetEffectOffset = Spirit.GetEffectOffset
Spirit_GetSpiritAmount = Spirit.GetSpiritAmount
Spirit_GetSpiritTypeID = Spirit.GetSpiritTypeID
Spirit_GetSpiritTipsInfo = Spirit.GetSpiritTipsInfo
Spirit_GetSpiritTipsOffset = Spirit.GetSpiritTipsOffset
Spirit_GetPrizeInfo = Spirit.GetPrizeInfo
Spirit_GetPrizeInfoEx = Spirit.GetPrizeInfoEx
Spirit_GetDlgInfo = Spirit.GetDlgInfo
Spirit_GetEffectByQLTotalLevel = Spirit.GetEffectByQLTotalLevel
Spirit_GetQLEffectMinLev = Spirit.GetQLEffectMinLev
Spirit_GetReforgeInfo = Spirit.GetReforgeInfo
Spirit_GetReforgeEffectInfo = Spirit.GetReforgeEffectInfo

return Spirit