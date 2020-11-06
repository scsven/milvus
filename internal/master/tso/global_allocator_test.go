package tso

import (
	"github.com/stretchr/testify/assert"
	"github.com/zilliztech/milvus-distributed/internal/conf"
	"github.com/zilliztech/milvus-distributed/internal/kv/mockkv"
	"github.com/zilliztech/milvus-distributed/internal/util/tsoutil"
	"os"
	"testing"
	"time"
)

var GTsoAllocator *GlobalTSOAllocator

func TestMain(m *testing.M) {
	GTsoAllocator = &GlobalTSOAllocator{
		timestampOracle: &timestampOracle{
			kvBase:        mockkv.NewEtcdKV(),
			rootPath:      conf.Config.Etcd.Rootpath,
			saveInterval:  3 * time.Second,
			maxResetTSGap: func() time.Duration { return 3 * time.Second },
			key:           "tso",
		},
	}
	exitCode := m.Run()
	os.Exit(exitCode)
}

func TestGlobalTSOAllocator_Initialize(t *testing.T) {
	err := GTsoAllocator.Initialize()
	assert.Nil(t, err)
}

func TestGlobalTSOAllocator_GenerateTSO(t *testing.T) {
	count := 1000
	perCount := uint32(100)
	startTs, err := GTsoAllocator.GenerateTSO(perCount)
	assert.Nil(t, err)
	lastPhysical, lastLogical := tsoutil.ParseTS(startTs)
	for i:=0;i < count; i++{
		ts, _ := GTsoAllocator.GenerateTSO(perCount)
		physical, logical := tsoutil.ParseTS(ts)
		if lastPhysical == physical {
			diff := logical - lastLogical
			assert.Equal(t, uint64(perCount), diff)
		}
		lastPhysical, lastLogical = physical, logical
	}
}

func TestGlobalTSOAllocator_SetTSO(t *testing.T) {
	curTime := time.Now()
	nextTime := curTime.Add(2 * time.Second )
	physical := nextTime.UnixNano() / int64(time.Millisecond)
	logical := int64(0)
	err := GTsoAllocator.SetTSO(tsoutil.ComposeTS(physical, logical))
	assert.Nil(t, err)
}

func TestGlobalTSOAllocator_UpdateTSO(t *testing.T) {
	err := GTsoAllocator.UpdateTSO()
	assert.Nil(t, err)
}

func TestGlobalTSOAllocator_Reset(t *testing.T) {
	GTsoAllocator.Reset()
}