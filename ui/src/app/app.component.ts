import {AfterViewInit, Component, ElementRef, OnInit, ViewChild} from '@angular/core';
import {GridStateService} from "./services/grid-state/grid-state.service";
import {BehaviorSubject, map, Observable} from "rxjs";
import {BeaconAtReceiverPipe} from "./pipes/beacon-at-receiver.pipe";
import {TimeElapsedService} from "./services/timeElapsed/time-elapsed.service";
import {ReceiverAliasService} from "./services/receiverAlias/receiver-alias.service";
import {HeartBeatService} from "./services/heartbeat/heart-beat.service";

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent implements OnInit, AfterViewInit {

  @ViewChild('table')
  table: ElementRef;

  @ViewChild('receivers')
  receivers: ElementRef

  timer$: Observable<number>
  columnNames = new BehaviorSubject<string[]>([])

  constructor(
    public gridState: GridStateService,
    public timeService: TimeElapsedService,
    public receiverAlias: ReceiverAliasService,
    public heartBeatServer: HeartBeatService
  ) {
  }

  ngOnInit(): void {
    this.gridState.receivers.pipe(
      map(names => ['Beacon', ...names])
    ).subscribe(this.columnNames)

    this.timer$ = this.timeService.time()
  }

  ngAfterViewInit(): void {
    this.gridState.receivers.pipe(
      map(names => names.length)
    )
      .subscribe(this.setNumberOfReceivers)
  }

  setNumberOfReceivers(num: number) {
    if (this.table == null) return
    this.table.nativeElement.style.gridTemplateColumns =
      'auto '.repeat(num)
    this.receivers.nativeElement.style.gridTemplateColumns =
      'auto '.repeat(num)
  }

  entries(obj: Object) {
    return Object.entries(obj)
  }

  changeAliasName(receiver: string, newName: string) {
    this.receiverAlias.setAlias(receiver, newName)
  }

}
